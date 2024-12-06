//TestFramework.cpp
#include "TestFramework.h"
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <queue>
#include <condition_variable>

void TestRunner::run(bool runConcurrently) {
    for (auto& suite : suites) {
        std::cout << "Running Test Suite: " << suite->name << std::endl;

        if (suite->fixture) {
            suite->fixture->BeforeAll();
        }

        std::mutex coutMutex;

        if (runConcurrently) {
            unsigned int numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) {
                numThreads = 2;
            }

            std::vector<std::thread> threadPool;
            std::queue<std::function<void()>> taskQueue;
            std::mutex queueMutex;
            std::condition_variable cv;
            bool done = false;

            auto worker = [&]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        cv.wait(lock, [&] { return done || !taskQueue.empty(); });
                        if (done && taskQueue.empty()) {
                            return;
                        }
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                    task();
                }
            };

            for (unsigned int i = 0; i < numThreads; ++i) {
                threadPool.emplace_back(worker);
            }

            for (auto& testCase : suite->testCases) {
                if (testCase.disabled) {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "Skipping Disabled Test Case: " << testCase.name << std::endl;
                    continue;
                }

                auto runTest = [&, testCase](int rep) {
                    if (suite->fixture) {
                        suite->fixture->BeforeEach();
                    }

                    {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "Running Test Case: " << testCase.name;
                        if (testCase.repetitions > 1) {
                            std::cout << " (Repetition " << rep << ")";
                        }
                        std::cout << std::endl;
                    }

                    bool exceptionCaught = false;
                    bool exceptionExpected = !testCase.expectedExceptionTypeName.empty();
                    bool testPassed = true;

                    auto executeTest = [&]() {
                        try {
                            testCase.function(suite->fixture.get(), rep);
                        } catch (const std::exception& e) {
                            exceptionCaught = true;
                            if (!exceptionExpected) {
                                std::lock_guard<std::mutex> lock(coutMutex);
                                std::cout << "Unexpected exception thrown in test '" << testCase.name << "': " << e.what() << std::endl;
                                testPassed = false;
                            } else if (std::string(typeid(e).name()) != testCase.expectedExceptionTypeName) {
                                std::lock_guard<std::mutex> lock(coutMutex);
                                std::cout << "Unexpected exception type in test '" << testCase.name << "': " << e.what() << std::endl;
                                testPassed = false;
                            }
                        } catch (...) {
                            exceptionCaught = true;
                            if (!exceptionExpected) {
                                std::lock_guard<std::mutex> lock(coutMutex);
                                std::cout << "Unexpected unknown exception thrown in test '" << testCase.name << "'" << std::endl;
                                testPassed = false;
                            }
                        }
                    };

                    if (testCase.timeout.count() > 0) {
                        std::future<void> future = std::async(std::launch::async, executeTest);
                        if (future.wait_for(testCase.timeout) == std::future_status::timeout) {
                            std::lock_guard<std::mutex> lock(coutMutex);
                            std::cout << "Test '" << testCase.name << "' timed out after " << testCase.timeout.count() << " ms" << std::endl;
                            testPassed = false;
                        } else {
                            future.get();
                        }
                    } else {
                        executeTest();
                    }

                    if (exceptionExpected && !exceptionCaught) {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "Expected exception of type '" << testCase.expectedExceptionTypeName << "' was not thrown in test '" << testCase.name << "'" << std::endl;
                        testPassed = false;
                    }

                    if (suite->fixture) {
                        suite->fixture->AfterEach();
                    }
                };

                if (testCase.isNondeterministic || testCase.repetitions > 1) {
                    for (int rep = 1; rep <= testCase.repetitions; ++rep) {
                        {
                            std::lock_guard<std::mutex> lock(queueMutex);
                            taskQueue.push([=] { runTest(rep); });
                        }
                        cv.notify_one();
                    }
                } else {
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        taskQueue.push([=] { runTest(1); });
                    }
                    cv.notify_one();
                }
            }

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                done = true;
            }
            cv.notify_all();

            for (auto& thread : threadPool) {
                thread.join();
            }
        } else {
            for (auto& testCase : suite->testCases) {
                if (testCase.disabled) {
                    std::cout << "Skipping Disabled Test Case: " << testCase.name << std::endl;
                    continue;
                }

                int repetitions = testCase.repetitions < 1 ? 1 : testCase.repetitions;
                for (int rep = 1; rep <= repetitions; ++rep) {
                    if (suite->fixture) {
                        suite->fixture->BeforeEach();
                    }

                    std::cout << "Running Test Case: " << testCase.name;
                    if (repetitions > 1) {
                        std::cout << " (Repetition " << rep << ")";
                    }
                    std::cout << std::endl;

                    bool exceptionCaught = false;
                    bool exceptionExpected = !testCase.expectedExceptionTypeName.empty();
                    bool testPassed = true;

                    auto executeTest = [&]() {
                        try {
                            testCase.function(suite->fixture.get(), rep);
                        } catch (const std::exception& e) {
                            exceptionCaught = true;
                            if (!exceptionExpected) {
                                std::cout << "Unexpected exception thrown in test '" << testCase.name << "': " << e.what() << std::endl;
                                testPassed = false;
                            } else if (std::string(typeid(e).name()) != testCase.expectedExceptionTypeName) {
                                std::cout << "Unexpected exception type in test '" << testCase.name << "': " << e.what() << std::endl;
                                testPassed = false;
                            }
                        } catch (...) {
                            exceptionCaught = true;
                            if (!exceptionExpected) {
                                std::cout << "Unexpected unknown exception thrown in test '" << testCase.name << "'" << std::endl;
                                testPassed = false;
                            }
                        }
                    };

                    if (testCase.timeout.count() > 0) {
                        std::future<void> future = std::async(std::launch::async, executeTest);
                        if (future.wait_for(testCase.timeout) == std::future_status::timeout) {
                            std::cout << "Test '" << testCase.name << "' timed out after " << testCase.timeout.count() << " ms" << std::endl;
                            testPassed = false;
                        } else {
                            future.get();
                        }
                    } else {
                        executeTest();
                    }

                    if (exceptionExpected && !exceptionCaught) {
                        std::cout << "Expected exception of type '" << testCase.expectedExceptionTypeName << "' was not thrown in test '" << testCase.name << "'" << std::endl;
                        testPassed = false;
                    }

                    if (suite->fixture) {
                        suite->fixture->AfterEach();
                    }
                }
            }
        }

        if (suite->fixture) {
            suite->fixture->AfterAll();
        }

        std::cout << std::endl;
    }
}

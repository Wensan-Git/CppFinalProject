// TestFramework.cpp
#include "TestFramework.h"
#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <queue>
#include <condition_variable>

TestSuite::TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance)
        : name(suiteName), fixture(fixtureInstance) {}

void TestSuite::addTestCase(const TestCase& testCase) {
    testCases.push_back(testCase);
}

TestRunner& TestRunner::getInstance() {
    static TestRunner instance;
    return instance;
}

void TestRunner::addTestSuite(std::shared_ptr<TestSuite> suite) {
    suites.push_back(suite);
}

void TestRunner::run(bool runConcurrently) {
    for (auto& suite : suites) {
        std::cout << "Running Test Suite: " << suite->name << std::endl;

        if (suite->fixture) {
            suite->fixture->BeforeAll();
        }

        std::mutex coutMutex;

        if (runConcurrently) {
            // Determine the number of hardware threads (CPU cores)
            unsigned int numThreads = std::thread::hardware_concurrency();
            if (numThreads == 0) {
                numThreads = 2; // Fallback to 2 threads if hardware_concurrency can't determine
            }

            // Create a thread pool
            std::vector<std::thread> threadPool;
            std::queue<std::function<void()>> taskQueue;
            std::mutex queueMutex;
            std::condition_variable cv;
            bool done = false;

            // Worker function for threads
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

            // Start threads
            for (unsigned int i = 0; i < numThreads; ++i) {
                threadPool.emplace_back(worker);
            }

            // Enqueue test cases as tasks
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
                        // Run test with timeout
                        std::future<void> future = std::async(std::launch::async, executeTest);
                        if (future.wait_for(testCase.timeout) == std::future_status::timeout) {
                            std::lock_guard<std::mutex> lock(coutMutex);
                            std::cout << "Test '" << testCase.name << "' timed out after " << testCase.timeout.count() << " ms" << std::endl;
                            testPassed = false;
                        } else {
                            future.get();
                        }
                    } else {
                        // Run test normally
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
                    // Enqueue each repetition as a separate task
                    for (int rep = 1; rep <= testCase.repetitions; ++rep) {
                        {
                            std::lock_guard<std::mutex> lock(queueMutex);
                            taskQueue.push([=] { runTest(rep); });
                        }
                        cv.notify_one();
                    }
                } else {
                    // Enqueue the test case as a single task
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        taskQueue.push([=] { runTest(1); });
                    }
                    cv.notify_one();
                }
            }

            // Signal threads to finish when tasks are done
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                done = true;
            }
            cv.notify_all();

            // Join threads
            for (auto& thread : threadPool) {
                thread.join();
            }
        } else {
            // Sequential execution
            for (auto& testCase : suite->testCases) {
                if (testCase.disabled) {
                    std::lock_guard<std::mutex> lock(coutMutex);
                    std::cout << "Skipping Disabled Test Case: " << testCase.name << std::endl;
                    continue;
                }

                int repetitions = testCase.repetitions;
                if (repetitions < 1) repetitions = 1;

                for (int rep = 1; rep <= repetitions; ++rep) {
                    if (suite->fixture) {
                        suite->fixture->BeforeEach();
                    }

                    {
                        std::lock_guard<std::mutex> lock(coutMutex);
                        std::cout << "Running Test Case: " << testCase.name;
                        if (repetitions > 1) {
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
                        // Run test with timeout
                        std::future<void> future = std::async(std::launch::async, executeTest);
                        if (future.wait_for(testCase.timeout) == std::future_status::timeout) {
                            std::lock_guard<std::mutex> lock(coutMutex);
                            std::cout << "Test '" << testCase.name << "' timed out after " << testCase.timeout.count() << " ms" << std::endl;
                            testPassed = false;
                        } else {
                            future.get();
                        }
                    } else {
                        // Run test normally
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
                }
            }
        }

        if (suite->fixture) {
            suite->fixture->AfterAll();
        }

        std::cout << std::endl;
    }
}

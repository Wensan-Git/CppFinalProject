// TestFramework.cpp
#include "TestFramework.h"
#include <iostream>
#include <thread>
#include <future>

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

void TestRunner::run() {
    for (auto& suite : suites) {
        std::cout << "Running Test Suite: " << suite->name << std::endl;

        if (suite->fixture) {
            suite->fixture->BeforeAll();
        }

        for (auto& testCase : suite->testCases) {
            if (testCase.disabled) {
                std::cout << "Skipping Disabled Test Case: " << testCase.name << std::endl;
                continue;
            }

            for (int rep = 0; rep < testCase.repetitions; ++rep) {
                if (suite->fixture) {
                    suite->fixture->BeforeEach();
                }

                std::cout << "Running Test Case: " << testCase.name;
                if (testCase.repetitions > 1) {
                    std::cout << " (Repetition " << (rep + 1) << ")";
                }
                std::cout << std::endl;

                bool exceptionCaught = false;
                bool exceptionExpected = !testCase.expectedExceptionTypeName.empty();
                bool testPassed = true;

                if (testCase.timeout.count() > 0) {
                    // Run with timeout
                    std::packaged_task<void()> task([&]() {
                        try {
                            testCase.function(suite->fixture.get());
                        } catch (...) {
                            exceptionCaught = true;
                            if (!exceptionExpected) {
                                std::cout << "Unexpected exception thrown in test '" << testCase.name << "'" << std::endl;
                            }
                        }
                    });
                    std::future<void> future = task.get_future();
                    std::thread taskThread(std::move(task));
                    if (future.wait_for(testCase.timeout) == std::future_status::timeout) {
                        std::cout << "Test '" << testCase.name << "' timed out after " << testCase.timeout.count() << " ms" << std::endl;
                        testPassed = false;
                        // It's hard to kill a thread in C++, so we detach it
                        taskThread.detach();
                    } else {
                        taskThread.join();
                        if (exceptionExpected && !exceptionCaught) {
                            std::cout << "Expected exception of type '" << testCase.expectedExceptionTypeName << "' was not thrown in test '" << testCase.name << "'" << std::endl;
                            testPassed = false;
                        }
                    }
                } else {
                    // Run normally
                    try {
                        testCase.function(suite->fixture.get());
                    } catch (...) {
                        exceptionCaught = true;
                        if (!exceptionExpected) {
                            std::cout << "Unexpected exception thrown in test '" << testCase.name << "'" << std::endl;
                            testPassed = false;
                        }
                    }
                    if (exceptionExpected && !exceptionCaught) {
                        std::cout << "Expected exception of type '" << testCase.expectedExceptionTypeName << "' was not thrown in test '" << testCase.name << "'" << std::endl;
                        testPassed = false;
                    }
                }

                if (suite->fixture) {
                    suite->fixture->AfterEach();
                }

                // Handle testPassed if needed
            }
        }

        if (suite->fixture) {
            suite->fixture->AfterAll();
        }

        std::cout << std::endl;
    }
}

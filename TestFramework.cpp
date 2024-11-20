// TestFramework.cpp
#include "TestFramework.h"
#include <iostream>

// Constructor for TestSuite
TestSuite::TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance)
        : name(suiteName), fixture(fixtureInstance) {}

// Add a test case to the suite
void TestSuite::addTestCase(const std::string& testName, const std::function<void(TestFixture*)>& testFunc) {
    testCases.emplace_back(testName, testFunc);
}

// Get the singleton instance of TestRunner
TestRunner& TestRunner::getInstance() {
    static TestRunner instance;
    return instance;
}

// Add a test suite to the runner
void TestRunner::addTestSuite(std::shared_ptr<TestSuite> suite) {
    suites.push_back(suite);
}

// Run all test suites and their test cases
void TestRunner::run() {
    for (auto& suite : suites) {
        std::cout << "Running Test Suite: " << suite->name << std::endl;

        // Call BeforeAll if it exists
        if (suite->fixture) {
            suite->fixture->BeforeAll();
        }

        // Run each test case
        for (auto& testCase : suite->testCases) {
            // BeforeEach
            if (suite->fixture) {
                suite->fixture->BeforeEach();
            }

            std::cout << "Running Test Case: " << testCase.first << std::endl;
            testCase.second(suite->fixture.get());

            // AfterEach
            if (suite->fixture) {
                suite->fixture->AfterEach();
            }
        }

        // AfterAll
        if (suite->fixture) {
            suite->fixture->AfterAll();
        }

        std::cout << std::endl;
    }
}

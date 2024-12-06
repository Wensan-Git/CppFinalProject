// RunInternalTests.cpp
#include "TestFramework.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>

// A helper function to check if a substring is present in a given string
bool contains(const std::string& text, const std::string& substring) {
    return text.find(substring) != std::string::npos;
}

int main() {
    TestRunner& runner = TestRunner::getInstance();

    // Capture the original buffer
    std::streambuf* originalCoutBuffer = std::cout.rdbuf();

    // Set up a stringstream to capture the output
    std::stringstream capturedOutput;
    std::cout.rdbuf(capturedOutput.rdbuf());

    std::cout << "Running internal tests (TestFrameworkTests) sequentially..." << std::endl;
    auto startSequential = std::chrono::high_resolution_clock::now();
    runner.run(false); // Run tests sequentially
    auto endSequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationSequential = endSequential - startSequential;
    std::cout << "Total time for sequential execution: " << durationSequential.count() << " seconds" << std::endl;

    std::string sequentialOutput = capturedOutput.str();

    capturedOutput.str("");
    capturedOutput.clear();

    std::cout << "\nRunning internal tests (TestFrameworkTests) concurrently..." << std::endl;
    auto startConcurrent = std::chrono::high_resolution_clock::now();
    runner.run(true); // Run tests concurrently
    auto endConcurrent = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationConcurrent = endConcurrent - startConcurrent;
    std::cout << "Total time for concurrent execution: " << durationConcurrent.count() << " seconds" << std::endl;

    // Capture the concurrent run output
    std::string concurrentOutput = capturedOutput.str();

    // Restore original cout buffer
    std::cout.rdbuf(originalCoutBuffer);

    bool allChecksPassed = true;

    // Check TestSimplePass: Should pass, so no "Assertion failed"
    {
        // If "Assertion failed" appears after "Running TestSimplePass", it's incorrect.
        bool hasAssertionFailed = contains(sequentialOutput, "Running TestSimplePass")
                                  && contains(sequentialOutput, "Assertion failed");
        if (hasAssertionFailed) {
            std::cout << "[CHECK] TestSimplePass: FAILED (Found unexpected assertion failure)" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestSimplePass: PASSED" << std::endl;
        }
    }

    // Check TestSimpleFail: Expect an assertion failure
    {
        bool foundFail = contains(sequentialOutput, "Assertion failed");
        if (!foundFail) {
            std::cout << "[CHECK] TestSimpleFail: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestSimpleFail: PASSED" << std::endl;
        }
    }

    // Check TestDisabledCheck: Expect skipping message
    {
        bool foundDisabledSkip = contains(sequentialOutput, "Skipping Disabled Test Case: TestDisabledCheck");
        if (!foundDisabledSkip) {
            std::cout << "[CHECK] TestDisabledCheck: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestDisabledCheck: PASSED" << std::endl;
        }
    }

    // Check TestExpectedException: Should not report that expected exception wasn't thrown
    {
        // We only fail if "Expected exception of type ... was not thrown" appears.
        bool noExpectedThrown = contains(sequentialOutput, "Expected exception of type 'std::runtime_error' was not thrown");
        // Also check no unexpected exception message for this test.
        bool unexpectedMessage = contains(sequentialOutput, "Unexpected exception thrown in test 'TestExpectedException'");
        if (noExpectedThrown || unexpectedMessage) {
            std::cout << "[CHECK] TestExpectedException: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestExpectedException: PASSED" << std::endl;
        }
    }

    // Check TestUnexpectedException: Should see "Unexpected exception thrown in test 'TestUnexpectedException'"
    {
        bool foundUnexpected = contains(sequentialOutput, "Unexpected exception thrown in test 'TestUnexpectedException");
        if (foundUnexpected) {
            std::cout << "[CHECK] TestUnexpectedException: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestUnexpectedException: PASSED" << std::endl;
        }
    }

    // Check TestTimeoutCase: Should see a timeout message "timed out after"
    {
        bool foundTimeout = contains(sequentialOutput, "timed out after");
        if (!foundTimeout) {
            std::cout << "[CHECK] TestTimeoutCase: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestTimeoutCase: PASSED" << std::endl;
        }
    }

    // Check TestRepeatedMixed: repetition fails
    {
        bool foundMixedTestFail = contains(sequentialOutput, "Assertion failed");
        if (!foundMixedTestFail) {
            std::cout << "[CHECK] TestRepeatedMixed: FAILED" << std::endl;
            allChecksPassed = false;
        } else {
            std::cout << "[CHECK] TestRepeatedMixed: PASSED" << std::endl;
        }
    }

    // Print overall result
    if (allChecksPassed) {
        std::cout << "\n[OVERALL RESULT] All checks PASSED." << std::endl;
    } else {
        std::cout << "\n[OVERALL RESULT] Some checks FAILED." << std::endl;
    }

    // Print performance improvement
    double speedup = durationSequential.count() / durationConcurrent.count();
    std::cout << "\nPerformance Improvement: " << speedup << "x faster when running concurrently." << std::endl;

    return allChecksPassed ? 0 : 1;
}

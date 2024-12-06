# CUnit++

## Introduction
This C++ TestFramework is a lightweight, macro-driven testing framework designed to simplify the creation, organization, and execution of unit tests. It provides an intuitive syntax for defining test suites and test cases, as well as powerful features such as timeouts, concurrency, mocks, and expected exception handling.

## Problem Setup
When developing applications in C++, ensuring code correctness through automated testing is crucial. This framework addresses the need for a structured, maintainable test environment. It allows developers to quickly define tests, run them in parallel (if desired), and verify correctness using built-in assertions and mock functionality. With support for fixture setup/teardown methods and various test types (e.g., disabled tests, timeout tests, repeated tests), it streamlines the testing process from start to finish.

## Overview of Functionalities

**Core Concepts:**
- **TestFixture**: A base class that lets you define `BeforeAll`, `AfterAll`, `BeforeEach`, and `AfterEach` methods to set up and tear down resources for related tests.
- **TestSuite**: A container for multiple test cases that share a common fixture, allowing you to logically group tests for the same component or module.
- **TestCase**: A single test that you define within a test suite; it can be configured to run multiple times, run concurrently, expect certain exceptions, or respect timeouts.
- **TestRunner**: A singleton manager that collects all registered test suites and runs their test cases, optionally enabling concurrency.

**Key Functionalities:**
- **`TEST_SUITE(suiteName)`**: Declares a new test suite and creates a corresponding fixture class. Use this to group related tests and define suite-level setup/teardown logic.
- **`REGISTER_TEST_SUITE(suiteName)`**: Registers the test suite with the test runner so that it can be discovered and executed. Include this after defining your test suite.
- **`BEFORE_ALL(suiteName)` / `AFTER_ALL(suiteName)`**: Defines methods that run once before and after all tests in the suite. Use these for global setup and cleanup tasks.
- **`BEFORE_EACH(suiteName)` / `AFTER_EACH(suiteName)`**: Defines methods that run before and after each individual test in the suite. Use these to prepare or reset state specific to each test.
- **`TEST_CASE(suiteName, testName)`**: Declares a single test function inside the specified suite. It is a basic building block for verifying code correctness.
- **`DISABLED_TEST_CASE(suiteName, testName)`**: Declares a test function that will not be executed. This is useful for temporarily bypassing tests without deleting them.
- **`CONCURRENT_TEST_CASE(suiteName, testName)`**: Declares a test that can be run in parallel when concurrency is enabled. Use this to speed up large test suites by utilizing multiple CPU cores.
- **`EXPECT_EXCEPTION_TEST_CASE(suiteName, testName, exceptionType)`**: Declares a test that must throw the specified exception to pass. Use this to verify error conditions and exception handling behavior.
- **`TIMEOUT_TEST_CASE(suiteName, testName, timeoutMs)`**: Declares a test that must complete within a given time limit. Use this to detect and fail long-running or stalled tests.
- **`REPEATED_TEST_CASE(suiteName, testName, repetitions)`**: Declares a test that will run multiple times. Use this to check for flaky tests or confirm behavior under repeated execution.
- **`Mock` and `MOCK_METHOD`**: Allows you to define mock objects and record method calls. Use these to isolate and verify interactions with dependencies.
- **`ASSERT_TRUE(condition)` / `ASSERT_EQ(expected, actual)`**: Assertion macros for verifying test conditions. Use these to detect and report failures clearly.
- **Concurrency Support**: By calling `run(true)` on the test runner, tests designated as concurrent can be run in parallel. Use this to reduce total testing time.
- **Timeout and Exception Handling**: Optional per-test timeouts and expected exceptions help ensure that tests remain responsive and accurately capture intended failure modes.

## Manual

### Installation
1. Place `TestFramework.h`, `TestFramework.cpp`, and all your test files (e.g., `MyTests.cpp`) in the same directory.
2. Compile them together into your executable using your preferred compiler.

### Usage
1. Define a test suite using `TEST_SUITE(MySuite)` and `REGISTER_TEST_SUITE(MySuite)`.
2. Within the suite, add test cases (e.g., `TEST_CASE(MySuite, MyTest)`).
3. Optionally define `BEFORE_ALL`, `AFTER_ALL`, `BEFORE_EACH`, and `AFTER_EACH` for setup and teardown at suite or test levels.
4. Run all tests by calling `TestRunner::getInstance().run();` in your `main` function.
    - Use `run(true)` to enable concurrent execution.
### Example

Below are some example snippets showing various features and utilizing some key functionalities of the framework:

```cpp
TEST_SUITE(MySuite) {
    // This fixture can hold shared data for all tests in MySuite
};

REGISTER_TEST_SUITE(MySuite);

BEFORE_ALL(MySuite) {
    // Runs once before any test in MySuite
    std::cout << "Setting up resources for all tests.\n";
}

AFTER_ALL(MySuite) {
    // Runs once after all tests in MySuite have finished
    std::cout << "Cleaning up resources.\n";
}
```
And if we want to run test cases concurrently:
```cpp
CONCURRENT_TEST_CASE(MySuite, ConcurrentTest) {
    // This test can be run in parallel with other tests if concurrency is enabled.
    ASSERT_TRUE(2 + 2 == 4);
}
```
If we want to use mocking to verify interaction:
```cpp
class MyMock : public Mock {
public:
    MOCK_METHOD(DoSomething, int, (int), ( (std::to_string(arg1)) ));
};

TEST_CASE(MySuite, MockingExample) {
    MyMock mockObject;
    // Specify the mock's behavior
    mockObject.DoSomething_mock = [](int x) { return x * 2; };

    // Call the mocked method and assert the outcome
    ASSERT_EQ(10, mockObject.DoSomething(5));

    // Verify that the mocked method was called with the expected argument
    ASSERT_TRUE(verifyCall(mockObject, "DoSomething", {"5"}));
}
```
To run the code:
```cpp
int main() {
    // Enable concurrency: if MySuite has concurrent tests, they will run in parallel
    TestRunner::getInstance().run(true);
    return 0;
}
```

# Files and Compilation

### Files:
- **TestFramework.h / TestFramework.cpp**: Core framework files providing test infrastructure, macros, test runner, and assertions.
- **MyTests.cpp / DemoTests.cpp**: Contains tests that you write to verify the correctness of your own application's logic and functionalities. These tests illustrate how you would use the framework in practice, targeting the functions and classes you develop.
- **TestFrameworkTests.cpp**: Contains internal tests designed to confirm that the testing framework itself behaves as expected. Instead of verifying your application code, these tests ensure that the framework correctly handles scenarios such as passing/failing tests, timeouts, exceptions, repeated tests, and disabled tests. In other words, they validate the robustness and reliability of the testing system itself.

- **main.cpp**: Entry point that runs all tests (from both `MyTests.cpp` and `TestFrameworkTests.cpp`) sequentially and then concurrently, measuring performance and demonstrating the impact of parallel execution.
- **RunInternalTests.cpp**: A separate entry point that focuses on running the internal framework tests (`TestFrameworkTests.cpp`) sequentially and concurrently, capturing output and verifying that the framework's own output and error handling remain consistent.


## How to Compile and Run
Assuming all files are in the same folder and using a common compiler (like `g++`):


### Running RunInternalTests.cpp (for internal unit tests)
```bash
g++ -std=c++11 -pthread -o run_internal RunInternalTests.cpp TestFramework.cpp MyTests.cpp
./run_internal
```

### Running demo_main.cpp (for demo)
```bash
 g++ -o demo demo_main.cpp DemoTests.cpp TestFramework.cpp -pthread -std=c++11
./demo
```

### Running main.cpp (for performance measurement)
```bash
g++ -std=c++11 -pthread -o run_main main.cpp TestFramework.cpp MyTests.cpp
./run_main
```

## Citation:
The Macros (ideas and structures) were inspired by GPT's magical power
We also consulted with C++ Manual Devdocs and Professor Stroustrup's A Tour of C++

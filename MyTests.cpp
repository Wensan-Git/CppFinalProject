// MyTests.cpp
#include "TestFramework.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>
#include <stdexcept>

// Define the test suite and fixture
TEST_SUITE(ArrayTestSuite) {
public:
    std::vector<int> largeArray; // Shared state within the fixture

    void BeforeAll() override;
    void AfterAll() override;
    void BeforeEach() override;
    void AfterEach() override;
};

// Implement the BeforeAll function
BEFORE_ALL(ArrayTestSuite) {
    std::cout << "Executing BeforeAll: Setting up large array." << std::endl;
    // Initialize a large array with 1 million integers
    largeArray.resize(1'000'000);
    std::iota(largeArray.begin(), largeArray.end(), 1); // Fill with values from 1 to 1,000,000
}

// Implement the AfterAll function
AFTER_ALL(ArrayTestSuite) {
    std::cout << "Executing AfterAll: Clearing large array." << std::endl;
    largeArray.clear();
}

// Implement the BeforeEach function
BEFORE_EACH(ArrayTestSuite) {
    std::cout << "Executing BeforeEach: Before test case." << std::endl;
    // Any setup before each test
}

// Implement the AfterEach function
AFTER_EACH(ArrayTestSuite) {
    std::cout << "Executing AfterEach: After test case." << std::endl;
    // Any cleanup after each test
}

// Register the test suite after the fixture class is fully defined
REGISTER_TEST_SUITE(ArrayTestSuite);

// Define test cases

// Regular test case
TEST_CASE(ArrayTestSuite, TestArraySum) {
    std::cout << "In TestArraySum" << std::endl;
    long long expectedSum = static_cast<long long>(fixture->largeArray.size()) * (fixture->largeArray.size() + 1) / 2;
    long long actualSum = std::accumulate(fixture->largeArray.begin(), fixture->largeArray.end(), 0LL);
    ASSERT_EQ(expectedSum, actualSum);
}

// Disabled test case
DISABLED_TEST_CASE(ArrayTestSuite, DisabledTest) {
    std::cout << "This test should not run." << std::endl;
    ASSERT_TRUE(false); // This assertion should not be reached
}

// Timeout test case
TIMEOUT_TEST_CASE(ArrayTestSuite, TestTimeout, 1000) {
    std::cout << "In TestTimeout" << std::endl;
    // Simulate long-running operation
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ASSERT_TRUE(true); // This assertion may not be reached if timeout occurs
}

// Repeated test case
REPEATED_TEST_CASE(ArrayTestSuite, TestRepeated, 3) {
    std::cout << "In TestRepeated, repetition " << repetition << std::endl;
    ASSERT_TRUE(repetition >= 1 && repetition <= 3);
}

// Expect exception test case
EXPECT_EXCEPTION_TEST_CASE(ArrayTestSuite, TestExpectException, std::out_of_range) {
    std::cout << "In TestExpectException" << std::endl;
    // This should throw std::out_of_range
    int value = fixture->largeArray.at(fixture->largeArray.size()); // Out of range
    (void)value; // Suppress unused variable warning
}

// Other test cases remain the same...

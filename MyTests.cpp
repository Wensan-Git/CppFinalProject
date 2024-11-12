// MyTests.cpp
#include "TestFramework.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>

// Note, this is a very very very initial version of our code. We have added explanation as detailed as possible for Mike to understand. Might be bugs,
// But Wensannnnnn brother will fix it!


// Define the test suite and fixture
TEST_SUITE(ArrayTestSuite) {
public:
    std::vector<int> largeArray; // Shared state within the fixture

    void BeforeAll() override;
};

// Implement the BeforeAll function
BEFORE_ALL(ArrayTestSuite) {
    std::cout << "Executing BeforeAll: Setting up large array." << std::endl;
    // Initialize a large array with 1 million integers
    largeArray.resize(1'000'000);
    std::iota(largeArray.begin(), largeArray.end(), 1); // Fill with values from 1 to 1,000,000
}

// Define test cases

// Test case to verify the sum of the array
TEST_CASE(ArrayTestSuite, TestArraySum) {
    std::cout << "In TestArraySum" << std::endl;
    long long expectedSum = static_cast<long long>(fixture->largeArray.size()) * (fixture->largeArray.size() + 1) / 2;
    long long actualSum = std::accumulate(fixture->largeArray.begin(), fixture->largeArray.end(), 0LL);
    ASSERT_EQ(expectedSum, actualSum);
}

// Test case to verify that the array is sorted
TEST_CASE(ArrayTestSuite, TestArraySorted) {
    std::cout << "In TestArraySorted" << std::endl;
    bool isSorted = std::is_sorted(fixture->largeArray.begin(), fixture->largeArray.end());
    ASSERT_TRUE(isSorted);
}

// Test case to reverse the array and verify the last element
TEST_CASE(ArrayTestSuite, TestArrayReverse) {
    std::cout << "In TestArrayReverse" << std::endl;
    std::reverse(fixture->largeArray.begin(), fixture->largeArray.end());
    ASSERT_EQ(1, fixture->largeArray.back()); // After reversing, last element should be 1
    // Restore the original order for other tests
    std::reverse(fixture->largeArray.begin(), fixture->largeArray.end());
}

// Test case to perform binary search
TEST_CASE(ArrayTestSuite, TestArrayBinarySearch) {
    std::cout << "In TestArrayBinarySearch" << std::endl;
    // Search for a value in the array
    bool found = std::binary_search(fixture->largeArray.begin(), fixture->largeArray.end(), 500'000);
    ASSERT_TRUE(found);
}

// Test case to modify the array and verify changes
TEST_CASE(ArrayTestSuite, TestArrayModify) {
    std::cout << "In TestArrayModify" << std::endl;
    // Modify the first 100 elements
    for (int i = 0; i < 100; ++i) {
        fixture->largeArray[i] *= 2;
    }
    // Verify that the first 100 elements have been doubled
    bool modificationCorrect = true;
    for (int i = 0; i < 100; ++i) {
        if (fixture->largeArray[i] != (i + 1) * 2) {
            modificationCorrect = false;
            break;
        }
    }
    ASSERT_TRUE(modificationCorrect);
    // Restore original values
    for (int i = 0; i < 100; ++i) {
        fixture->largeArray[i] /= 2;
    }
}

// Test case to measure performance (illustrative)
TEST_CASE(ArrayTestSuite, TestArrayPerformance) {
    std::cout << "In TestArrayPerformance" << std::endl;
    // Measure the time to sum the array
    auto start = std::chrono::high_resolution_clock::now();
    long long sum = std::accumulate(fixture->largeArray.begin(), fixture->largeArray.end(), 0LL);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time to sum array: " << duration.count() << " seconds" << std::endl;
    // No assertion here, just output
}

// Test case to sum array elements using multiple threads
#include <thread>

TEST_CASE(ArrayTestSuite, TestArrayMultiThreadSum) {
    std::cout << "In TestArrayMultiThreadSum" << std::endl;
    const size_t numThreads = 4;
    std::vector<std::thread> threads;
    std::vector<long long> partialSums(numThreads, 0);

    size_t chunkSize = fixture->largeArray.size() / numThreads;

    // Launch threads to compute partial sums
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            size_t startIdx = i * chunkSize;
            size_t endIdx = (i == numThreads - 1) ? fixture->largeArray.size() : startIdx + chunkSize;
            partialSums[i] = std::accumulate(fixture->largeArray.begin() + startIdx, fixture->largeArray.begin() + endIdx, 0LL);
        });
    }

    // Wait for threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Sum up the partial sums
    long long totalSum = std::accumulate(partialSums.begin(), partialSums.end(), 0LL);
    long long expectedSum = static_cast<long long>(fixture->largeArray.size()) * (fixture->largeArray.size() + 1) / 2;

    ASSERT_EQ(expectedSum, totalSum);
}

// MyTests.cpp
#include "TestFramework.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <thread>
#include <stdexcept>
#include <mutex>
#include <cmath>
#include <random>
#include <string>
#include <sstream>



// Helper functions first
long long computeLargePrime(int n) {
    int count = 0;
    long long num = 2;
    while (count < n) {
        bool isPrime = true;
        for (long long i = 2; i <= std::sqrt(num); ++i) {
            if (num % i == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            ++count;
        }
        ++num;
    }
    return num - 1;
}

long long computeFactorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

double computeIntegral(double a, double b, int n) {
    auto f = [](double x) { return std::sin(x) * std::exp(-x); };
    double h = (b - a) / n;
    double sum = 0.5 * (f(a) + f(b));
    for (int i = 1; i < n; ++i) {
        sum += f(a + i * h);
    }
    return h * sum;
}

// Define the test suite and fixture
TEST_SUITE(HeavyComputationTestSuite) {
public:
    void BeforeAll() override;
    void AfterAll() override;
    void BeforeEach() override;
    void AfterEach() override;

    std::mutex resourceMutex;
    int sharedCounter = 0;
};

// Register the test suite
REGISTER_TEST_SUITE(HeavyComputationTestSuite);

// Implement fixture methods
BEFORE_ALL(HeavyComputationTestSuite) {
    std::cout << "Executing BeforeAll: Setting up resources." << std::endl;
}

AFTER_ALL(HeavyComputationTestSuite) {
    std::cout << "Executing AfterAll: Cleaning up resources." << std::endl;
}

BEFORE_EACH(HeavyComputationTestSuite) {
}

AFTER_EACH(HeavyComputationTestSuite) {
}

// Regular test cases
CONCURRENT_TEST_CASE(HeavyComputationTestSuite, TestComputePrime1) {
    std::cout << "In TestComputePrime1" << std::endl;
    int n = 5000;
    long long prime = computeLargePrime(n);
    ASSERT_TRUE(prime > 0);
}

CONCURRENT_TEST_CASE(HeavyComputationTestSuite, TestComputePrime2) {
    std::cout << "In TestComputePrime2" << std::endl;
    int n = 5000;
    long long prime = computeLargePrime(n);
    ASSERT_TRUE(prime > 0);
}

TEST_CASE(HeavyComputationTestSuite, TestComputePrimeSequential) {
    std::cout << "In TestComputePrimeSequential" << std::endl;
    int n = 5000;
    long long prime = computeLargePrime(n);
    ASSERT_TRUE(prime > 0);
}

DISABLED_TEST_CASE(HeavyComputationTestSuite, TestDisabled) {
    std::cout << "This test should not run." << std::endl;
    ASSERT_TRUE(false);
}

EXPECT_EXCEPTION_TEST_CASE(HeavyComputationTestSuite, TestExpectException, std::runtime_error) {
    std::cout << "In TestExpectException" << std::endl;
    throw std::runtime_error("Expected exception");
}

TIMEOUT_TEST_CASE(HeavyComputationTestSuite, TestTimeout, 500) {
    std::cout << "In TestTimeout" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_TRUE(true);
}

// Define the nondeterministic test case
void HeavyComputationTestSuite_TestNondeterministic(HeavyComputationTestSuite_Fixture* fixture, int repetition) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(100, 300);
    int delay = dist(rng);

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    {
        std::lock_guard<std::mutex> lock(fixture->resourceMutex);
        fixture->sharedCounter++;
    }

    std::cout << "Repetition " << repetition << ": Slept for " << delay << " ms, Counter = " << fixture->sharedCounter << std::endl;
    ASSERT_TRUE(true);
}

// Register the nondeterministic test case
static struct HeavyComputationTestSuite_TestNondeterministic_Registrar {
    HeavyComputationTestSuite_TestNondeterministic_Registrar() {
        TestCase testCase("TestNondeterministic",
                          [](TestFixture* baseFixture, int repetition) {
                              HeavyComputationTestSuite_TestNondeterministic(
                                      static_cast<HeavyComputationTestSuite_Fixture*>(baseFixture),
                                      repetition
                              );
                          }
        );
        testCase.repetitions = 5;
        testCase.isNondeterministic = true;
        HeavyComputationTestSuite->addTestCase(testCase);
    }
} HeavyComputationTestSuite_TestNondeterministic_registrar;

// Define the repeated test case
void HeavyComputationTestSuite_TestRepeated(HeavyComputationTestSuite_Fixture* fixture, int repetition) {
    std::cout << "In TestRepeated, Repetition " << repetition << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(true);
}

// Register the repeated test case
static struct HeavyComputationTestSuite_TestRepeated_Registrar {
    HeavyComputationTestSuite_TestRepeated_Registrar() {
        TestCase testCase("TestRepeated",
                          [](TestFixture* baseFixture, int repetition) {
                              HeavyComputationTestSuite_TestRepeated(
                                      static_cast<HeavyComputationTestSuite_Fixture*>(baseFixture),
                                      repetition
                              );
                          }
        );
        testCase.repetitions = 3;
        HeavyComputationTestSuite->addTestCase(testCase);
    }
} HeavyComputationTestSuite_TestRepeated_registrar;


// More complex mocked class
class AdvancedCalculator {
public:
    virtual ~AdvancedCalculator() = default;
    virtual int add3(int a, int b, int c) {
        return a + b + c;
    }
    virtual std::string concatStrings(const std::string& s1, const std::string& s2, const std::string& s3) {
        return s1 + s2 + s3;
    }
    virtual double multiplyMany(double a, double b, double c, double d) {
        return a * b * c * d;
    }
    virtual void noArgsMethod() { }
};

class MockAdvancedCalculator : public AdvancedCalculator, public Mock {
public:
    MOCK_METHOD(add3, int, (int a, int b, int c), (a, b, c));
    MOCK_METHOD(concatStrings, std::string, (const std::string& s1, const std::string& s2, const std::string& s3), (s1, s2, s3));
    MOCK_METHOD(multiplyMany, double, (double a, double b, double c, double d), (a, b, c, d));
    MOCK_METHOD(noArgsMethod, void, (), ());
};

TEST_CASE(HeavyComputationTestSuite, TestAdvancedMockingAdd3) {
    std::cout << "In TestAdvancedMockingAdd3" << std::endl;

    MockAdvancedCalculator mockCalc;
    mockCalc.add3_mock = [](int a, int b, int c) {
        return a + b + c + 10;
    };

    int result = mockCalc.add3(1, 2, 3);
    ASSERT_TRUE(verifyCall(mockCalc, "add3", {"1", "2", "3"}));
    ASSERT_EQ(16, result);
}

TEST_CASE(HeavyComputationTestSuite, TestAdvancedMockingConcatStrings) {
    std::cout << "In TestAdvancedMockingConcatStrings" << std::endl;

    MockAdvancedCalculator mockCalc;
    mockCalc.concatStrings_mock = [](const std::string& s1, const std::string& s2, const std::string& s3) {
        return s1 + "-" + s2 + "-" + s3;
    };

    std::string result = mockCalc.concatStrings("Hello", "Mock", "World");
    ASSERT_TRUE(verifyCall(mockCalc, "concatStrings", {"Hello", "Mock", "World"}));
    ASSERT_EQ("Hello-Mock-World", result);
}

TEST_CASE(HeavyComputationTestSuite, TestAdvancedMockingMultiplyMany) {
    std::cout << "In TestAdvancedMockingMultiplyMany" << std::endl;

    MockAdvancedCalculator mockCalc;
    mockCalc.multiplyMany_mock = [](double a, double b, double c, double d) {
        return (a * b * c * d) + 5.0;
    };

    double result = mockCalc.multiplyMany(2.0, 3.0, 4.0, 5.0);
    ASSERT_TRUE(verifyCall(mockCalc, "multiplyMany", {"2", "3", "4", "5"}));
    ASSERT_EQ(125.0, result);
}

TEST_CASE(HeavyComputationTestSuite, TestAdvancedMockingNoArgsMethod) {
    std::cout << "In TestAdvancedMockingNoArgsMethod" << std::endl;

    MockAdvancedCalculator mockCalc;
    bool called = false;
    mockCalc.noArgsMethod_mock = [&]() {
        called = true;
    };

    mockCalc.noArgsMethod();
    ASSERT_TRUE(verifyCall(mockCalc, "noArgsMethod", {}));
    ASSERT_TRUE(called);
}


// ArrayTestSuite
TEST_SUITE(ArrayTestSuite) {
public:
    std::vector<int> largeArray;

    void BeforeAll() override;
    void BeforeEach() override;
};

REGISTER_TEST_SUITE(ArrayTestSuite);

// Implement the BeforeAll function
BEFORE_ALL(ArrayTestSuite) {
    std::cout << "Executing BeforeAll: Setting up large array (once)." << std::endl;
    // We can remove the initialization here if we want, or leave it as is.
    // It won't matter since we will re-initialize in BeforeEach anyway.
    largeArray.resize(1000000);
    std::iota(largeArray.begin(), largeArray.end(), 1);
}

// Add the BeforeEach function to ensure each test starts with a clean array.
BEFORE_EACH(ArrayTestSuite) {
    // Re-initialize largeArray for each test to ensure a clean, sorted state.
    largeArray.clear();
    largeArray.resize(1000000);
    std::iota(largeArray.begin(), largeArray.end(), 1);
}

// Now we can rely on largeArray always being {1, 2, 3, ..., 1000000} at the start of each test.

// Test case to reverse the array and verify the last element
TEST_CASE(ArrayTestSuite, TestArrayReverse) {
    std::cout << "In TestArrayReverse" << std::endl;
    std::reverse(fixture->largeArray.begin(), fixture->largeArray.end());
    ASSERT_EQ(1, fixture->largeArray.back()); // After reversing, last element should be 1
    // No need to restore the array now since BeforeEach handles re-initialization for the next test.
}

// Test case to perform binary search
TEST_CASE(ArrayTestSuite, TestArrayBinarySearch) {
    std::cout << "In TestArrayBinarySearch" << std::endl;
    // largeArray is guaranteed to be in ascending order.
    bool found = std::binary_search(fixture->largeArray.begin(), fixture->largeArray.end(), 500000);
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
    // No need to restore, BeforeEach will reset for the next test.
}

// Test case to measure performance (illustrative)
TEST_CASE(ArrayTestSuite, TestArrayPerformance) {
    std::cout << "In TestArrayPerformance" << std::endl;
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
            partialSums[i] = std::accumulate(fixture->largeArray.begin() + startIdx,
                                             fixture->largeArray.begin() + endIdx, 0LL);
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
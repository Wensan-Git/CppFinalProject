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
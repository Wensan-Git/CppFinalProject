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

// Global variables to control test complexity and counts
static int g_primeTestCount = 5000;         // Controls complexity of prime tests
static int g_numLightTests = 10;            // Controls how many trivial tests to register
static int g_numModerateTests = 5;          // Controls how many moderate factorial tests to register
static int g_numHeavyTests = 3;             // Controls how many heavy prime tests to register

extern void setPrimeTestCount(int n) {
    g_primeTestCount = n;
}

// You can define similar setters for the number of tests:
extern void setNumLightTests(int n) {
    g_numLightTests = n;
}
extern void setNumModerateTests(int n) {
    g_numModerateTests = n;
}
extern void setNumHeavyTests(int n) {
    g_numHeavyTests = n;
}

// Helper functions
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

// ------------------------------
// Existing Test Suites
// ------------------------------

// First test suite
TEST_SUITE(HeavyComputationTestSuite) {
public:
    void BeforeAll() override;
    void AfterAll() override;
    void BeforeEach() override;
    void AfterEach() override;

    std::mutex resourceMutex;
    int sharedCounter = 0;
};

REGISTER_TEST_SUITE(HeavyComputationTestSuite);

BEFORE_ALL(HeavyComputationTestSuite) {
    std::cout << "Executing BeforeAll: Setting up resources for HeavyComputationTestSuite." << std::endl;
}

AFTER_ALL(HeavyComputationTestSuite) {
    std::cout << "Executing AfterAll: Cleaning up resources for HeavyComputationTestSuite." << std::endl;
}

BEFORE_EACH(HeavyComputationTestSuite) {
}

AFTER_EACH(HeavyComputationTestSuite) {
}

/**
 * @brief Compute the g_primeTestCount-th prime concurrently and verify it's positive.
 */
CONCURRENT_TEST_CASE(HeavyComputationTestSuite, TestComputePrime1) {
    std::cout << "In TestComputePrime1 with prime count: " << g_primeTestCount << std::endl;
    long long prime = computeLargePrime(g_primeTestCount);
    ASSERT_TRUE(prime > 0);
}

/**
 * @brief Another concurrent heavy prime computation test, using g_primeTestCount.
 */
CONCURRENT_TEST_CASE(HeavyComputationTestSuite, TestComputePrime2) {
    std::cout << "In TestComputePrime2 with prime count: " << g_primeTestCount << std::endl;
    long long prime = computeLargePrime(g_primeTestCount);
    ASSERT_TRUE(prime > 0);
}

/**
 * @brief Sequential baseline prime computation test to compare with concurrent runs.
 */
TEST_CASE(HeavyComputationTestSuite, TestComputePrimeSequential) {
    std::cout << "In TestComputePrimeSequential with prime count: " << g_primeTestCount << std::endl;
    long long prime = computeLargePrime(g_primeTestCount);
    ASSERT_TRUE(prime > 0);
}

/**
 * @brief Disabled test should never run.
 */
DISABLED_TEST_CASE(HeavyComputationTestSuite, TestDisabled) {
    std::cout << "This test should not run." << std::endl;
    ASSERT_TRUE(false);
}

/**
 * @brief Test expecting a runtime_error exception.
 */
EXPECT_EXCEPTION_TEST_CASE(HeavyComputationTestSuite, TestExpectException, std::runtime_error) {
    std::cout << "In TestExpectException" << std::endl;
    throw std::runtime_error("Expected exception");
}

/**
 * @brief Test that will timeout because it sleeps longer than allowed.
 */
TIMEOUT_TEST_CASE(HeavyComputationTestSuite, TestTimeout, 500) {
    std::cout << "In TestTimeout" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_TRUE(true);
}

// Nondeterministic test case
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

// Register the nondeterministic test
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

// Repeated test case
void HeavyComputationTestSuite_TestRepeated(HeavyComputationTestSuite_Fixture* fixture, int repetition) {
    std::cout << "In TestRepeated, Repetition " << repetition << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(true);
}

// Register the repeated test
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


// Mock-related tests
class AdvancedCalculator {
public:
    virtual ~AdvancedCalculator() = default;
    virtual int add3(int a, int b, int c) { return a + b + c; }
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

/**
 * @brief Tests mocking add3 to add an extra 10.
 */
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

/**
 * @brief Tests mocking concatStrings to insert dashes instead of direct concatenation.
 */
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

/**
 * @brief Tests mocking multiplyMany to add 5.0 to the product.
 */
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

/**
 * @brief Tests mocking noArgsMethod to just set a boolean flag.
 */
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


// ------------------------------
// Additional test suite to show more failing and heavy tests
// ------------------------------
TEST_SUITE(AdditionalHeavyTests) {
public:
    void BeforeAll() override;
    void AfterAll() override;
    void BeforeEach() override;
    void AfterEach() override;

    std::mutex testMutex;
    int sharedValue = 0;
};

REGISTER_TEST_SUITE(AdditionalHeavyTests);

BEFORE_ALL(AdditionalHeavyTests) {
    std::cout << "BeforeAll in AdditionalHeavyTests: Setting up." << std::endl;
}

AFTER_ALL(AdditionalHeavyTests) {
    std::cout << "AfterAll in AdditionalHeavyTests: Tearing down." << std::endl;
}

BEFORE_EACH(AdditionalHeavyTests) {
    std::cout << "BeforeEach in AdditionalHeavyTests: Resetting sharedValue." << std::endl;
    sharedValue = 0;
}

AFTER_EACH(AdditionalHeavyTests) {
    std::cout << "AfterEach in AdditionalHeavyTests." << std::endl;
}

/**
 * @brief Intentionally fails by checking a wrong factorial result.
 */
TEST_CASE(AdditionalHeavyTests, TestFactorialMismatch) {
    std::cout << "In TestFactorialMismatch" << std::endl;
    long long fact = computeFactorial(10);
    ASSERT_EQ(9999999, fact); // Wrong expected result to ensure failure
}

/**
 * @brief Concurrent increments on a shared variable without proper synchronization.
 */
CONCURRENT_TEST_CASE(AdditionalHeavyTests, TestConcurrentIncrements) {
    std::cout << "In TestConcurrentIncrements" << std::endl;
    for (int i = 0; i < 100000; ++i) {
        fixture->sharedValue++;
    }
    ASSERT_TRUE(fixture->sharedValue >= 0);
}

/**
 * @brief A test that times out intentionally.
 */
TIMEOUT_TEST_CASE(AdditionalHeavyTests, TestLongRunningComputation, 500) {
    std::cout << "In TestLongRunningComputation" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    ASSERT_TRUE(true);
}

/**
 * @brief Test expecting a logic_error.
 */
EXPECT_EXCEPTION_TEST_CASE(AdditionalHeavyTests, TestThrowLogicError, std::logic_error) {
    std::cout << "In TestThrowLogicError" << std::endl;
    throw std::logic_error("Deliberate logic_error thrown");
}

/**
 * @brief Disabled test that should never run.
 */
DISABLED_TEST_CASE(AdditionalHeavyTests, TestDisabledCheck) {
    std::cout << "This test should not run." << std::endl;
    ASSERT_TRUE(false);
}

/**
 * @brief Compute integral and check range.
 */
TEST_CASE(AdditionalHeavyTests, TestApproxIntegral) {
    std::cout << "In TestApproxIntegral" << std::endl;
    double result = computeIntegral(0, 10, 100000);
    ASSERT_TRUE(result > 0.45 && result < 0.5);
}

/**
 * @brief Intentionally fail.
 */
TEST_CASE(AdditionalHeavyTests, TestFailOnPurpose) {
    std::cout << "In TestFailOnPurpose" << std::endl;
    ASSERT_TRUE(false);
}


// ------------------------------
// New Test Suites for Performance Measurement
// ------------------------------

// 1) ManyLightTestsSuite: a large number of trivial tests (controlled by g_numLightTests)
TEST_SUITE(ManyLightTestsSuite) {
public:
    void BeforeAll() override { std::cout << "BeforeAll in ManyLightTestsSuite" << std::endl; }
    void AfterAll() override { std::cout << "AfterAll in ManyLightTestsSuite" << std::endl; }
    void BeforeEach() override {}
    void AfterEach() override {}
};

REGISTER_TEST_SUITE(ManyLightTestsSuite);

// Dynamically create many trivial tests
static struct ManyLightTests_Registrar {
    ManyLightTests_Registrar() {
        for (int i = 0; i < g_numLightTests; ++i) {
            std::string testName = "LightTest_" + std::to_string(i);
            TestCase testCase(testName, [](TestFixture*, int) {
                ASSERT_TRUE(true);
            });
            ManyLightTestsSuite->addTestCase(testCase);
        }
    }
} ManyLightTests_registrar;


// 2) ModerateTestsSuite: a moderate number of tests performing factorial computations
TEST_SUITE(ModerateTestsSuite) {
public:
    void BeforeAll() override { std::cout << "BeforeAll in ModerateTestsSuite" << std::endl; }
    void AfterAll() override { std::cout << "AfterAll in ModerateTestsSuite" << std::endl; }
    void BeforeEach() override {}
    void AfterEach() override {}
};

REGISTER_TEST_SUITE(ModerateTestsSuite);

static struct ModerateTests_Registrar {
    ModerateTests_Registrar() {
        for (int i = 0; i < g_numModerateTests; ++i) {
            std::string testName = "FactorialTest_" + std::to_string(i);
            // Each test computes factorial(10000) for moderate complexity
            TestCase testCase(testName, [](TestFixture*, int) {
                long long fact = computeFactorial(10000);
                ASSERT_TRUE(fact > 0); // Just a sanity check
            });
            ModerateTestsSuite->addTestCase(testCase);
        }
    }
} ModerateTests_registrar;


// 3) HeavyPrimeTestsSuite: a few heavy tests each computing a large prime (controlled by g_primeTestCount and g_numHeavyTests)
TEST_SUITE(HeavyPrimeTestsSuite) {
public:
    void BeforeAll() override { std::cout << "BeforeAll in HeavyPrimeTestsSuite" << std::endl; }
    void AfterAll() override { std::cout << "AfterAll in HeavyPrimeTestsSuite" << std::endl; }
    void BeforeEach() override {}
    void AfterEach() override {}
};

REGISTER_TEST_SUITE(HeavyPrimeTestsSuite);

static struct HeavyPrimeTests_Registrar {
    HeavyPrimeTests_Registrar() {
        for (int i = 0; i < g_numHeavyTests; ++i) {
            std::string testName = "HeavyPrimeTest_" + std::to_string(i);
            // Each test computes a large prime with g_primeTestCount
            TestCase testCase(testName, [](TestFixture*, int) {
                long long prime = computeLargePrime(g_primeTestCount);
                ASSERT_TRUE(prime > 0);
            });
            HeavyPrimeTestsSuite->addTestCase(testCase);
        }
    }
} HeavyPrimeTests_registrar;

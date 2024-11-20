// TestFramework.h
#ifndef TESTFRAMEWORK_H
#define TESTFRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <cstdio> // For printf
#include <typeinfo>

// Base class for test fixtures
class TestFixture {
public:
    virtual ~TestFixture() = default;
    virtual void BeforeAll() {}
    virtual void AfterAll() {}
    virtual void BeforeEach() {}
    virtual void AfterEach() {}
};

// Struct representing a test case with additional properties
class TestCase {
public:
    std::string name;
    std::function<void(TestFixture*)> function;
    bool disabled = false;
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero();
    int repetitions = 1;
    std::string expectedExceptionTypeName;

    TestCase(const std::string& name,
             const std::function<void(TestFixture*)>& function)
        : name(name), function(function) {}
};

// Class representing a test suite
class TestSuite {
public:
    std::string name;
    std::shared_ptr<TestFixture> fixture;
    std::vector<TestCase> testCases;

    TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance);

    void addTestCase(const TestCase& testCase);
};

// Singleton TestRunner to manage and execute test suites
class TestRunner {
public:
    static TestRunner& getInstance();
    void addTestSuite(std::shared_ptr<TestSuite> suite);
    void run();

private:
    std::vector<std::shared_ptr<TestSuite>> suites;
    TestRunner() = default; // Private constructor for singleton
};

// Macros to simplify test suite and test case definitions
#define TEST_SUITE(suiteName) \
    class suiteName##_Fixture : public TestFixture

#define REGISTER_TEST_SUITE(suiteName) \
    std::shared_ptr<TestSuite> suiteName = std::make_shared<TestSuite>(#suiteName, std::make_shared<suiteName##_Fixture>()); \
    static struct suiteName##_Registrar { \
        suiteName##_Registrar() { \
            TestRunner::getInstance().addTestSuite(suiteName); \
        } \
    } suiteName##_registrar;

#define BEFORE_ALL(suiteName) \
    void suiteName##_Fixture::BeforeAll()

#define AFTER_ALL(suiteName) \
    void suiteName##_Fixture::AfterAll()

#define BEFORE_EACH(suiteName) \
    void suiteName##_Fixture::BeforeEach()

#define AFTER_EACH(suiteName) \
    void suiteName##_Fixture::AfterEach()

// Regular test case
#define TEST_CASE(suiteName, testName) \
    void suiteName##_##testName(suiteName##_Fixture* fixture); \
    static struct suiteName##_##testName##_Registrar { \
        suiteName##_##testName##_Registrar() { \
            suiteName->addTestCase(TestCase(#testName, [](TestFixture* baseFixture) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture)); \
            })); \
        } \
    } suiteName##_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture)

// Disabled test case
#define DISABLED_TEST_CASE(suiteName, testName) \
    void suiteName##_##testName(suiteName##_Fixture* fixture); \
    static struct suiteName##_DISABLED_##testName##_Registrar { \
        suiteName##_DISABLED_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture)); \
            }); \
            testCase.disabled = true; \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_DISABLED_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture)

// Timeout test case
#define TIMEOUT_TEST_CASE(suiteName, testName, timeoutMs) \
    void suiteName##_##testName(suiteName##_Fixture* fixture); \
    static struct suiteName##_TIMEOUT_##testName##_Registrar { \
        suiteName##_TIMEOUT_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture)); \
            }); \
            testCase.timeout = std::chrono::milliseconds(timeoutMs); \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_TIMEOUT_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture)

// Repeated test case
#define REPEATED_TEST_CASE(suiteName, testName, repetitions) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition); \
    static struct suiteName##_REPEAT_##testName##_Registrar { \
        suiteName##_REPEAT_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture) { \
                auto fixture = static_cast<suiteName##_Fixture*>(baseFixture); \
                for (int i = 1; i <= (repetitions); ++i) { \
                    fixture->BeforeEach(); \
                    suiteName##_##testName(fixture, i); \
                    fixture->AfterEach(); \
                } \
            }); \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_REPEAT_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

// Expect exception test case
#define EXPECT_EXCEPTION_TEST_CASE(suiteName, testName, exceptionType) \
    void suiteName##_##testName(suiteName##_Fixture* fixture); \
    static struct suiteName##_EXCEPTION_##testName##_Registrar { \
        suiteName##_EXCEPTION_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture) { \
                bool exceptionCaught = false; \
                try { \
                    suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture)); \
                } catch (const exceptionType&) { \
                    exceptionCaught = true; \
                } \
                if (!exceptionCaught) { \
                    printf("Expected exception of type '%s' was not thrown in test '%s'\n", #exceptionType, #testName); \
                } \
            }); \
            testCase.expectedExceptionTypeName = #exceptionType; \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_EXCEPTION_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture)

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        printf("Assertion failed in %s at line %d: %s\n", __FILE__, __LINE__, #condition); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        printf("Assertion failed in %s at line %d: Expected %s == %s\n", __FILE__, __LINE__, #expected, #actual); \
    }

#endif // TESTFRAMEWORK_H

// TestFramework.h
#ifndef TESTFRAMEWORK_H
#define TESTFRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdio> // For printf

// Base class for test fixtures
class TestFixture {
public:
    virtual ~TestFixture() = default;
    virtual void BeforeAll() {}
    virtual void AfterAll() {}
    virtual void BeforeEach() {}
    virtual void AfterEach() {}
};

// Class representing a test suite
class TestSuite {
public:
    std::string name;
    std::shared_ptr<TestFixture> fixture;
    std::vector<std::pair<std::string, std::function<void(TestFixture*)>>> testCases;

    TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance);

    void addTestCase(const std::string& testName, const std::function<void(TestFixture*)>& testFunc);
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

#define TEST_CASE(suiteName, testName) \
    void suiteName##_##testName(suiteName##_Fixture* fixture); \
    static struct suiteName##_##testName##_Registrar { \
        suiteName##_##testName##_Registrar() { \
            suiteName->addTestCase(#testName, [](TestFixture* baseFixture) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture)); \
            }); \
        } \
    } suiteName##_##testName##_registrar; \
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

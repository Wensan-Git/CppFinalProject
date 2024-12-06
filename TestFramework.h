#ifndef TESTFRAMEWORK_H
#define TESTFRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <cstdio>
#include <typeinfo>
#include <mutex>
#include <future>
#include <sstream>

// Base class for test fixtures
class TestFixture {
public:
    virtual ~TestFixture() = default;
    virtual void BeforeAll() {}
    virtual void AfterAll() {}
    virtual void BeforeEach() {}
    virtual void AfterEach() {}
};

// Struct representing a test case
class TestCase {
public:
    std::string name;
    std::function<void(TestFixture*, int)> function;
    bool disabled = false;
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero();
    int repetitions = 1;
    std::string expectedExceptionTypeName;
    bool concurrent = false;
    bool isNondeterministic = false;

    TestCase(const std::string& name,
             const std::function<void(TestFixture*, int)>& function)
        : name(name), function(function) {}
};

// Class representing a test suite
class TestSuite {
public:
    std::string name;
    std::shared_ptr<TestFixture> fixture;
    std::vector<TestCase> testCases;

    TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance)
        : name(suiteName), fixture(fixtureInstance) {}

    void addTestCase(const TestCase& testCase) {
        testCases.push_back(testCase);
    }
};

// Singleton TestRunner
class TestRunner {
public:
    static TestRunner& getInstance() {
        static TestRunner instance;
        return instance;
    }

    void addTestSuite(std::shared_ptr<TestSuite> suite) {
        suites.push_back(suite);
    }

    void run(bool runConcurrently = false);

private:
    std::vector<std::shared_ptr<TestSuite>> suites;
    TestRunner() = default;
};

// Assertion macros
#undef ASSERT_TRUE
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cout << "Assertion failed in " << __FILE__ << " at line " << __LINE__ \
                  << ": " << #condition << std::endl; \
    }

#undef ASSERT_EQ
#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cout << "Assertion failed in " << __FILE__ << " at line " << __LINE__ \
                  << ": Expected " << (expected) << " == " << (actual) << std::endl; \
    }

// Helper to convert arguments to strings
template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Overload for std::string
inline std::string toString(const std::string& value) {
    return value;
}

// Variadic template to convert arbitrary arguments to a vector of strings
inline std::vector<std::string> argsToString() {
    return {};
}

template<typename T, typename... Args>
std::vector<std::string> argsToString(T&& first, Args&&... rest) {
    std::vector<std::string> result;
    result.push_back(toString(std::forward<T>(first)));
    auto tail = argsToString(std::forward<Args>(rest)...);
    result.insert(result.end(), tail.begin(), tail.end());
    return result;
}

// Base class for mocks
class Mock {
public:
    virtual ~Mock() = default;

    void clearExpectations() {
        callLog.clear();
    }

    void recordCall(const std::string& methodName, const std::vector<std::string>& args) {
        callLog.push_back({methodName, args});
    }

    struct CallInfo {
        std::string methodName;
        std::vector<std::string> args;
    };

    std::vector<CallInfo> callLog;
};

// Functions to verify calls on mocks
inline bool verifyCall(Mock& mock, const std::string& methodName, const std::vector<std::string>& expectedArgs) {
    for (const auto& call : mock.callLog) {
        if (call.methodName == methodName && call.args == expectedArgs) {
            return true;
        }
    }
    return false;
}

inline int getCallCount(Mock& mock, const std::string& methodName) {
    int count = 0;
    for (const auto& call : mock.callLog) {
        if (call.methodName == methodName) {
            count++;
        }
    }
    return count;
}


// Test suite macros
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
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition = 1); \
    static struct suiteName##_##testName##_Registrar { \
        suiteName##_##testName##_Registrar() { \
            suiteName->addTestCase(TestCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            })); \
        } \
    } suiteName##_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define CONCURRENT_TEST_CASE(suiteName, testName) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition = 1); \
    static struct suiteName##_CONCURRENT_##testName##_Registrar { \
        suiteName##_CONCURRENT_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            }); \
            testCase.concurrent = true; \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_CONCURRENT_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define DISABLED_TEST_CASE(suiteName, testName) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition = 1); \
    static struct suiteName##_DISABLED_##testName##_Registrar { \
        suiteName##_DISABLED_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            }); \
            testCase.disabled = true; \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_DISABLED_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define EXPECT_EXCEPTION_TEST_CASE(suiteName, testName, exceptionType) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition = 1); \
    static struct suiteName##_EXCEPTION_##testName##_Registrar { \
        suiteName##_EXCEPTION_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            }); \
            testCase.expectedExceptionTypeName = #exceptionType; \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_EXCEPTION_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define TIMEOUT_TEST_CASE(suiteName, testName, timeoutMs) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition = 1); \
    static struct suiteName##_TIMEOUT_##testName##_Registrar { \
        suiteName##_TIMEOUT_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            }); \
            testCase.timeout = std::chrono::milliseconds(timeoutMs); \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_TIMEOUT_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define REPEATED_TEST_CASE(suiteName, testName, repetitions) \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition); \
    static struct suiteName##_REPEAT_##testName##_Registrar { \
        suiteName##_REPEAT_##testName##_Registrar() { \
            TestCase testCase(#testName, [](TestFixture* baseFixture, int repetition) { \
                suiteName##_##testName(static_cast<suiteName##_Fixture*>(baseFixture), repetition); \
            }); \
            testCase.repetitions = (repetitions); \
            suiteName->addTestCase(testCase); \
        } \
    } suiteName##_REPEAT_##testName##_registrar; \
    void suiteName##_##testName(suiteName##_Fixture* fixture, int repetition)

#define MOCK_METHOD(methodName, returnType, PARAMS, ARGS) \
    std::function<returnType PARAMS> methodName##_mock; \
    virtual returnType methodName PARAMS override { \
        std::vector<std::string> args = argsToString ARGS; \
        this->recordCall(#methodName, args); \
        if (methodName##_mock) \
            return methodName##_mock ARGS; \
        else \
            return returnType(); \
    }

#endif // TESTFRAMEWORK_H

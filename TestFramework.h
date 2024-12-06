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
/**
 * @brief A base fixture class that can be inherited by test suites to define shared setup/teardown logic.
 *
 * Override BeforeAll(), AfterAll(), BeforeEach(), and AfterEach() as needed to handle suite-level or test-level
 * resource management. By default, these methods do nothing.
 */
class TestFixture {
public:
    virtual ~TestFixture() = default;

    /**
     * @brief Called once before all tests in the suite execute.
     */
    virtual void BeforeAll() {}

    /**
     * @brief Called once after all tests in the suite have finished executing.
     */
    virtual void AfterAll() {}

    /**
     * @brief Called before each individual test in the suite runs.
     */
    virtual void BeforeEach() {}

    /**
     * @brief Called after each individual test in the suite finishes running.
     */
    virtual void AfterEach() {}
};

// Struct representing a test case
/**
 * @brief Represents a single test definition, including the test's name, the function to run,
 * its disabled/enabled state, timeout, expected exception type, concurrency flag, and repetition count.
 *
 * Instances are usually created automatically by the test case macros, and then added to a TestSuite.
 */
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

    /**
     * @brief Constructs a TestCase with the given name and test function.
     * @param name The name of the test case.
     * @param function A callable that takes a TestFixture pointer and repetition number, executing the test logic.
     */
    TestCase(const std::string& name,
             const std::function<void(TestFixture*, int)>& function)
            : name(name), function(function) {}
};

// Class representing a test suite
/**
 * @brief A collection of related test cases that share a common fixture.
 *
 * Each TestSuite holds a fixture instance and a list of TestCases. By grouping related tests, you can run setup
 * and teardown code once for all tests in that group, or before/after each test.
 */
class TestSuite {
public:
    std::string name;
    std::shared_ptr<TestFixture> fixture;
    std::vector<TestCase> testCases;

    /**
     * @brief Constructs a TestSuite with the specified name and fixture.
     * @param suiteName The name of the test suite.
     * @param fixtureInstance A shared pointer to a fixture object that will be used by the suite's tests.
     */
    TestSuite(const std::string& suiteName, std::shared_ptr<TestFixture> fixtureInstance)
            : name(suiteName), fixture(fixtureInstance) {}

    /**
     * @brief Adds a TestCase to the suite's collection of tests.
     * @param testCase The test case to be added.
     */
    void addTestCase(const TestCase& testCase) {
        testCases.push_back(testCase);
    }
};

// Singleton TestRunner
/**
 * @brief A singleton class responsible for managing and running all registered test suites.
 *
 * Use getInstance() to retrieve the global TestRunner. Add suites with addTestSuite(), and then call run() to
 * execute tests. By passing run(true), tests marked as concurrent can be run in parallel.
 */
class TestRunner {
public:
    /**
     * @brief Retrieves the global TestRunner instance.
     * @return A reference to the singleton TestRunner.
     */
    static TestRunner& getInstance() {
        static TestRunner instance;
        return instance;
    }

    /**
     * @brief Registers a test suite with the runner.
     * @param suite A shared pointer to the TestSuite being added.
     */
    void addTestSuite(std::shared_ptr<TestSuite> suite) {
        suites.push_back(suite);
    }

    /**
     * @brief Executes all registered test suites.
     * @param runConcurrently If true, eligible tests are run in parallel. Otherwise, all tests run sequentially.
     */
    void run(bool runConcurrently = false);

private:
    std::vector<std::shared_ptr<TestSuite>> suites;

    TestRunner() = default;
};

// Assertion macros
/**
 * @brief Asserts that a given condition is true.
 * Prints an error message if the assertion fails.
 * @param condition The boolean expression to verify.
 */
#undef ASSERT_TRUE
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        std::cout << "Assertion failed in " << __FILE__ << " at line " << __LINE__ \
                  << ": " << #condition << std::endl; \
    }

/**
 * @brief Asserts that two values are equal.
 * Prints an error message if the assertion fails.
 * @param expected The expected value.
 * @param actual The actual value obtained.
 */
#undef ASSERT_EQ
#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cout << "Assertion failed in " << __FILE__ << " at line " << __LINE__ \
                  << ": Expected " << (expected) << " == " << (actual) << std::endl; \
    }

// Helper to convert arguments to strings
/**
 * @brief Converts a value to its string representation.
 * @tparam T The type of the value.
 * @param value The value to convert.
 * @return A string representation of the input value.
 */
template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// Overload for std::string
/**
 * @brief Converts a std::string to itself, acting as a pass-through.
 * @param value A std::string value.
 * @return The same string.
 */
inline std::string toString(const std::string& value) {
    return value;
}

/**
 * @brief Base case for argsToString, returns an empty vector.
 * @return An empty vector of strings.
 */
inline std::vector<std::string> argsToString() {
    return {};
}

/**
 * @brief Variadic template function that converts multiple arguments into a vector of strings.
 * @tparam T The type of the first argument.
 * @tparam Args The types of the remaining arguments.
 * @param first The first argument.
 * @param rest The remaining arguments.
 * @return A vector of string representations of all arguments.
 */
template<typename T, typename... Args>
std::vector<std::string> argsToString(T&& first, Args&&... rest) {
    std::vector<std::string> result;
    result.push_back(toString(std::forward<T>(first)));
    auto tail = argsToString(std::forward<Args>(rest)...);
    result.insert(result.end(), tail.begin(), tail.end());
    return result;
}

// Base class for mocks
/**
 * @brief A base class for creating mock objects that record and verify method calls.
 *
 * Derive from this class and use MOCK_METHOD macros to define mocked methods. The calls are recorded in callLog,
 * which can be verified using verifyCall and getCallCount.
 */
class Mock {
public:
    virtual ~Mock() = default;

    /**
     * @brief Clears all recorded expectations (method calls).
     */
    void clearExpectations() {
        callLog.clear();
    }

    /**
     * @brief Records a method call on the mock object.
     * @param methodName The name of the method that was called.
     * @param args A vector of string representations of the arguments passed.
     */
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
/**
 * @brief Checks if a specified call with certain arguments was recorded on a mock.
 * @param mock The mock object to check.
 * @param methodName The method name to look for.
 * @param expectedArgs The expected arguments for the call.
 * @return True if the call was found, false otherwise.
 */
inline bool verifyCall(Mock& mock, const std::string& methodName, const std::vector<std::string>& expectedArgs) {
    for (const auto& call : mock.callLog) {
        if (call.methodName == methodName && call.args == expectedArgs) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Counts how many times a method was called on the mock.
 * @param mock The mock object to check.
 * @param methodName The method name to count occurrences of.
 * @return The number of times the method was called.
 */
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
/**
 * @brief Declares a new test suite and defines its fixture class.
 * @param suiteName The name of the test suite.
 */
#define TEST_SUITE(suiteName) \
    class suiteName##_Fixture : public TestFixture

/**
 * @brief Registers the test suite with the TestRunner so it can be discovered and executed.
 * @param suiteName The name of the test suite previously declared with TEST_SUITE.
 */
#define REGISTER_TEST_SUITE(suiteName) \
    std::shared_ptr<TestSuite> suiteName = std::make_shared<TestSuite>(#suiteName, std::make_shared<suiteName##_Fixture>()); \
    static struct suiteName##_Registrar { \
        suiteName##_Registrar() { \
            TestRunner::getInstance().addTestSuite(suiteName); \
        } \
    } suiteName##_registrar;

/**
 * @brief Defines a method to be run once before all tests in the specified suite.
 * @param suiteName The suite for which BeforeAll is being defined.
 */
#define BEFORE_ALL(suiteName) \
    void suiteName##_Fixture::BeforeAll()

/**
 * @brief Defines a method to be run once after all tests in the specified suite have completed.
 * @param suiteName The suite for which AfterAll is being defined.
 */
#define AFTER_ALL(suiteName) \
    void suiteName##_Fixture::AfterAll()

/**
 * @brief Defines a method to be run before each individual test in the specified suite.
 * @param suiteName The suite for which BeforeEach is being defined.
 */
#define BEFORE_EACH(suiteName) \
    void suiteName##_Fixture::BeforeEach()

/**
 * @brief Defines a method to be run after each individual test in the specified suite completes.
 * @param suiteName The suite for which AfterEach is being defined.
 */
#define AFTER_EACH(suiteName) \
    void suiteName##_Fixture::AfterEach()

/**
 * @brief Declares a single test case within the specified suite.
 * @param suiteName The suite in which to declare this test.
 * @param testName The name of the test case.
 */
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

/**
 * @brief Declares a single test case within the specified suite that can be run concurrently.
 * @param suiteName The suite in which to declare this concurrent test.
 * @param testName The name of the test case.
 */
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

/**
 * @brief Declares a test case that is disabled and will not run.
 * @param suiteName The suite in which to declare this disabled test.
 * @param testName The name of the test case.
 */
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

/**
 * @brief Declares a test case that expects a specific exception type to be thrown.
 * @param suiteName The suite in which to declare this test.
 * @param testName The name of the test case.
 * @param exceptionType The exception type that is expected to be thrown for the test to pass.
 */
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

/**
 * @brief Declares a test case that must complete within a specified timeout.
 * @param suiteName The suite in which to declare this test.
 * @param testName The name of the test case.
 * @param timeoutMs The time limit in milliseconds before the test is considered to have failed due to timeout.
 */
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

/**
 * @brief Declares a test case that runs multiple times (repetitions).
 * @param suiteName The suite in which to declare this test.
 * @param testName The name of the test case.
 * @param repetitions The number of times this test should be run.
 */
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

/**
 * @brief Declares a mock method inside a mock class, recording calls and allowing for custom return behavior.
 * @param methodName The name of the mocked method.
 * @param returnType The return type of the mocked method.
 * @param PARAMS The parameter list in parentheses for the mocked method.
 * @param ARGS The argument list in parentheses used when calling argsToString for logging.
 */
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

// main.cpp
#include "TestFramework.h"

int main() {
    // Run all registered test suites
    TestRunner::getInstance().run();
    return 0;
}

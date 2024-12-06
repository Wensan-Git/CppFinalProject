// demo_main.cpp
#include "TestFramework.h"
#include <iostream>
#include <chrono>

int main() {
    TestRunner& runner = TestRunner::getInstance();

    std::cout << "Running tests sequentially..." << std::endl;
    auto startSequential = std::chrono::high_resolution_clock::now();
    runner.run(false); // Run tests sequentially
    auto endSequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationSequential = endSequential - startSequential;
    std::cout << "Total time for sequential execution: " << durationSequential.count() << " seconds" << std::endl;

    std::cout << "\nRunning tests concurrently..." << std::endl;
    auto startConcurrent = std::chrono::high_resolution_clock::now();
    runner.run(true); // Run tests concurrently
    auto endConcurrent = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationConcurrent = endConcurrent - startConcurrent;
    std::cout << "Total time for concurrent execution: " << durationConcurrent.count() << " seconds" << std::endl;

    // Calculate and display the performance improvement
    double speedup = durationSequential.count() / durationConcurrent.count();
    std::cout << "\nPerformance Improvement: " << speedup << "x faster when running concurrently." << std::endl;

    return 0;
}
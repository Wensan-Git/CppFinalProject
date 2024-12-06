// main.cpp
#include "TestFramework.h"
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

// External setters defined in MyTests.cpp
extern void setPrimeTestCount(int n);
extern void setNumLightTests(int n);
extern void setNumModerateTests(int n);
extern void setNumHeavyTests(int n);

double runAndMeasure(TestRunner& runner, bool concurrent) {
    auto start = std::chrono::high_resolution_clock::now();
    runner.run(concurrent);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = end - start;
    return dur.count();
}

int main() {
    TestRunner& runner = TestRunner::getInstance();

    // -------------------------------
    // 1. Speedup vs Number of Tests
    // Vary number of light tests: {10, 100, 500, 1000}
    // Keep complexity fixed (primeTestCount = 10000)
    // -------------------------------
    {
        std::ofstream csv("speedup_vs_number_of_tests.csv");
        csv << "NumTests,SequentialTime,ConcurrentTime,Speedup\n";

        setPrimeTestCount(10000); // Fixed complexity for this experiment

        std::vector<int> testCounts = {10000, 20000, 50000, 100000, 200000};
        for (int count : testCounts) {
            setNumLightTests(count);
            setNumModerateTests(5);    // fixed small number
            setNumHeavyTests(3);       // fixed small number

            std::cout << "\n--- Testing Speedup vs Number of Tests: " << count << " tests ---\n";

            double seqTime = runAndMeasure(runner, false);
            double concTime = runAndMeasure(runner, true);
            double speedup = seqTime / concTime;

            csv << count << "," << seqTime << "," << concTime << "," << speedup << "\n";
        }

        csv.close();
        std::cout << "Results written to speedup_vs_number_of_tests.csv\n";
    }

    // -------------------------------
    // 2. Speedup vs Complexity
    // For complexity, we use primeTestCount.
    // -------------------------------
    {
        std::ofstream csv("speedup_vs_complexity.csv");
        csv << "ComplexityLevel,SequentialTime,ConcurrentTime,Speedup\n";

        // Let's fix number of tests moderately
        setNumLightTests(100);
        setNumModerateTests(10);
        setNumHeavyTests(5);

        // Complexity levels: maybe these represent increasing complexity in prime calculation
        std::vector<int> complexityLevels = {100000, 200000, 500000, 1000000, 2000000};
        for (int comp : complexityLevels) {
            setPrimeTestCount(comp);

            std::cout << "\n--- Testing Speedup vs Complexity: " << comp << " ---\n";

            double seqTime = runAndMeasure(runner, false);
            double concTime = runAndMeasure(runner, true);
            double speedup = seqTime / concTime;

            csv << comp << "," << seqTime << "," << concTime << "," << speedup << "\n";
        }

        csv.close();
        std::cout << "Results written to speedup_vs_complexity.csv\n";
    }

    std::cout << "All performance experiments completed.\n";
    return 0;
}

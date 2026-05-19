#pragma once

#include "BmiClassifier.h"
#include <array>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class SHealth {
public:
    SHealth();
    explicit SHealth(std::unique_ptr<BmiClassifier> classifier);

    int calculateBmi(const std::string& filename);
    double getBmiRatio(int ageClass, int type);

private:
    struct HealthRecord {
        int age = 0;
        double weight = 0.0;
        double height = 0.0;
        double bmi = 0.0;
    };

    struct RatioSet {
        std::array<double, 4> values{};
    };

    std::vector<HealthRecord> records;
    std::map<int, RatioSet> ratiosByAgeClass;
    std::unique_ptr<BmiClassifier> bmiClassifier;

    std::vector<std::string> split(const std::string& line, char delimiter);
    void resetState();
    std::optional<std::vector<HealthRecord>> loadRecords(const std::string& filename);
    std::optional<HealthRecord> parseRecord(const std::string& line);
    std::map<int, double> calculateAverageWeights() const;
    void imputeMissingWeights();
    void calculateBmiValues();
    std::map<int, RatioSet> aggregateRatios() const;
    RatioSet calculateRatioSet(int ageClass) const;
    int toAgeClass(int age) const;
    double percentage(int part, int total) const;
    double calculateBmiValue(double weight, double height) const;
    std::optional<BmiCategory> categoryFromType(int type) const;
};

#include "SHealth.h"
#include "StandardBmiClassifier.h"
#include <array>
#include <fstream>
#include <memory>
#include <map>
#include <optional>
#include <sstream>
#include <vector>

namespace {
constexpr char CsvDelimiter = ',';
constexpr int MinAgeClass = 20;
constexpr int MaxAgeClass = 70;
constexpr int AgeClassStep = 10;
constexpr int UnderweightCode = 100;
constexpr int NormalCode = 200;
constexpr int OverweightCode = 300;
constexpr int ObesityCode = 400;
constexpr double CentimetersPerMeter = 100.0;
constexpr std::size_t ExpectedColumnCount = 4;
constexpr std::size_t AgeColumn = 1;
constexpr std::size_t WeightColumn = 2;
constexpr std::size_t HeightColumn = 3;

const std::array<int, 6> AgeClasses = {20, 30, 40, 50, 60, 70};
} // namespace

SHealth::SHealth() : SHealth(std::make_unique<StandardBmiClassifier>()) {
}

SHealth::SHealth(std::unique_ptr<BmiClassifier> classifier)
    : bmiClassifier(std::move(classifier)) {
    if (!bmiClassifier) {
        bmiClassifier = std::make_unique<StandardBmiClassifier>();
    }
}

int SHealth::calculateBmi(const std::string& filename) {
    resetState();

    auto loadedRecords = loadRecords(filename);
    if (!loadedRecords) {
        return 0;
    }

    records = std::move(*loadedRecords);
    imputeMissingWeights();
    calculateBmiValues();
    ratiosByAgeClass = aggregateRatios();

    return static_cast<int>(records.size());
}

double SHealth::getBmiRatio(int ageClass, int type) {
    const auto category = categoryFromType(type);
    const auto ratioIt = ratiosByAgeClass.find(ageClass);
    if (!category || ratioIt == ratiosByAgeClass.end()) {
        return 0.0;
    }

    return ratioIt->second.values[static_cast<std::size_t>(*category)];
}

std::vector<std::string> SHealth::split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void SHealth::resetState() {
    records.clear();
    ratiosByAgeClass.clear();
}

std::optional<std::vector<SHealth::HealthRecord>> SHealth::loadRecords(
    const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::vector<HealthRecord> loadedRecords;
    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        auto record = parseRecord(line);
        if (record) {
            loadedRecords.push_back(*record);
        }
    }
    return loadedRecords;
}

std::optional<SHealth::HealthRecord> SHealth::parseRecord(const std::string& line) {
    const auto tokens = split(line, CsvDelimiter);
    if (tokens.size() < ExpectedColumnCount) {
        return std::nullopt;
    }

    try {
        HealthRecord record;
        record.age = std::stoi(tokens[AgeColumn]);
        record.weight = std::stod(tokens[WeightColumn]);
        record.height = std::stod(tokens[HeightColumn]);
        if (record.weight < 0.0 || record.height <= 0.0) {
            return std::nullopt;
        }
        return record;
    } catch (...) {
        return std::nullopt;
    }
}

std::map<int, double> SHealth::calculateAverageWeights() const {
    std::map<int, double> totals;
    std::map<int, int> counts;
    for (const auto& record : records) {
        const int ageClass = toAgeClass(record.age);
        if (ageClass == 0 || record.weight == 0.0) {
            continue;
        }
        totals[ageClass] += record.weight;
        counts[ageClass]++;
    }

    std::map<int, double> averages;
    for (const auto& [ageClass, total] : totals) {
        averages[ageClass] = total / counts[ageClass];
    }
    return averages;
}

void SHealth::imputeMissingWeights() {
    const auto averages = calculateAverageWeights();
    for (auto& record : records) {
        const int ageClass = toAgeClass(record.age);
        const auto average = averages.find(ageClass);
        if (record.weight == 0.0 && average != averages.end()) {
            record.weight = average->second;
        }
    }
}

void SHealth::calculateBmiValues() {
    for (auto& record : records) {
        record.bmi = calculateBmiValue(record.weight, record.height);
    }
}

std::map<int, SHealth::RatioSet> SHealth::aggregateRatios() const {
    std::map<int, RatioSet> ratios;
    for (const int ageClass : AgeClasses) {
        ratios[ageClass] = calculateRatioSet(ageClass);
    }
    return ratios;
}

SHealth::RatioSet SHealth::calculateRatioSet(int ageClass) const {
    RatioSet ratioSet;
    std::array<int, 4> counts{};
    int total = 0;

    for (const auto& record : records) {
        if (toAgeClass(record.age) != ageClass || record.bmi <= 0.0) {
            continue;
        }
        counts[static_cast<std::size_t>(bmiClassifier->classify(record.bmi))]++;
        total++;
    }

    for (std::size_t i = 0; i < counts.size(); ++i) {
        ratioSet.values[i] = percentage(counts[i], total);
    }
    return ratioSet;
}

int SHealth::toAgeClass(int age) const {
    if (age < MinAgeClass || age >= MaxAgeClass + AgeClassStep) {
        return 0;
    }
    return (age / AgeClassStep) * AgeClassStep;
}

double SHealth::percentage(int part, int total) const {
    if (total == 0) {
        return 0.0;
    }
    return static_cast<double>(part) * 100.0 / total;
}

double SHealth::calculateBmiValue(double weight, double height) const {
    if (weight <= 0.0 || height <= 0.0) {
        return 0.0;
    }
    const double heightInMeters = height / CentimetersPerMeter;
    return weight / (heightInMeters * heightInMeters);
}

std::optional<BmiCategory> SHealth::categoryFromType(int type) const {
    switch (type) {
    case UnderweightCode:
        return BmiCategory::Underweight;
    case NormalCode:
        return BmiCategory::Normal;
    case OverweightCode:
        return BmiCategory::Overweight;
    case ObesityCode:
        return BmiCategory::Obesity;
    default:
        return std::nullopt;
    }
}

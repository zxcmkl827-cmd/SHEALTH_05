#include <gtest/gtest.h>
#include "ApprovalTests.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#ifndef SHEALTH_GOLDEN_DATA_FILE
#define SHEALTH_GOLDEN_DATA_FILE "shealth.dat"
#endif

#ifndef SHEALTH_GOLDEN_EXECUTABLE_FILE
#define SHEALTH_GOLDEN_EXECUTABLE_FILE "SHealthBMI"
#endif

#ifndef SHEALTH_GOLDEN_WORKING_DIR
#define SHEALTH_GOLDEN_WORKING_DIR "."
#endif

#ifndef SHEALTH_GOLDEN_EXPECTED_DIR
#define SHEALTH_GOLDEN_EXPECTED_DIR "."
#endif

#include "SHealth.h"

namespace {
constexpr int UnderweightType = 100;
constexpr int NormalType = 200;
constexpr int OverweightType = 300;
constexpr int ObesityType = 400;

std::string normalizeLineEndings(std::string text) {
    std::string normalized;
    normalized.reserve(text.size());
    for (char ch : text) {
        if (ch != '\r') {
            normalized.push_back(ch);
        }
    }
    return normalized;
}

std::string trimTrailingNewlines(std::string text) {
    while (!text.empty() && text.back() == '\n') {
        text.pop_back();
    }
    return text;
}

std::string quote(const std::filesystem::path& path) {
    return "\"" + path.string() + "\"";
}

std::string runSHealthBmi(const std::filesystem::path& executable,
                          const std::filesystem::path& workingDir) {
#ifdef _WIN32
    const std::string command = "cd /d " + quote(workingDir) + " && " + quote(executable);
    FILE* pipe = _popen(command.c_str(), "r");
#else
    const std::string command = "cd " + quote(workingDir) + " && " + quote(executable);
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) {
        return {};
    }

    char buffer[256];
    std::ostringstream output;
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        output << buffer;
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return output.str();
}

std::filesystem::path writeFeatureGoldenMasterInput() {
    const auto path = std::filesystem::temp_directory_path() /
                      "shealth_feature_golden_master.csv";
    std::ofstream file(path);
    file << "id,age,weight,height\n";
    file << "under-20,20,59,180\n";
    file << "normal-20,21,65,180\n";
    file << "over-20,22,78,180\n";
    file << "obese-20,23,82,180\n";
    file << "height-imputed-normal,24,72,0\n";
    file << "unresolved-height-30,30,60,0\n";
    file << "outside-age-normal,80,65,180\n";
    return path;
}

std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
}

void appendRatioLine(std::ostringstream& output,
                     const std::string& label,
                     double underweight,
                     double normal,
                     double overweight,
                     double obesity) {
    output << label << " - underweight = " << underweight
           << ", normal = " << normal << ", overweight = " << overweight
           << ", obesity = " << obesity << '\n';
}

std::string buildFeatureSnapshot(const std::filesystem::path& inputFile) {
    SHealth health;
    const int recordCount = health.calculateBmi(inputFile.string());

    std::ostringstream output;
    output << std::fixed << std::setprecision(6);
    output << "Feature Golden Master\n";
    output << "records = " << recordCount << "\n\n";

    output << "Age class ratios\n";
    for (const int ageClass : {20, 30, 40, 50, 60, 70}) {
        appendRatioLine(output, std::to_string(ageClass),
                        health.getBmiRatio(ageClass, UnderweightType),
                        health.getBmiRatio(ageClass, NormalType),
                        health.getBmiRatio(ageClass, OverweightType),
                        health.getBmiRatio(ageClass, ObesityType));
    }

    output << "\nOverall ratios\n";
    output << "underweight = " << health.getOverallBmiRatio(UnderweightType)
           << '\n';
    output << "normal = " << health.getOverallBmiRatio(NormalType) << '\n';
    output << "overweight = " << health.getOverallBmiRatio(OverweightType)
           << '\n';
    output << "obesity = " << health.getOverallBmiRatio(ObesityType) << "\n\n";

    output << "Normal BMI users\n";
    for (const auto& user : health.getNormalBmiUsers()) {
        output << user.id << ", age = " << user.age
               << ", weight = " << user.weight << ", height = " << user.height
               << ", bmi = " << user.bmi << '\n';
    }

    return output.str();
}
} // namespace

TEST(TexttestFixture, SHealthBmiOutputMatchesGoldenMaster) {
    const auto dataFile = std::filesystem::path(SHEALTH_GOLDEN_DATA_FILE);
    const auto executable = std::filesystem::path(SHEALTH_GOLDEN_EXECUTABLE_FILE);
    const auto workingDir = std::filesystem::path(SHEALTH_GOLDEN_WORKING_DIR);

    ASSERT_TRUE(std::filesystem::exists(dataFile)) << dataFile.string();
    ASSERT_TRUE(std::filesystem::exists(executable)) << executable.string();
    ASSERT_TRUE(std::filesystem::exists(workingDir)) << workingDir.string();

    const std::string actual =
        trimTrailingNewlines(normalizeLineEndings(runSHealthBmi(executable, workingDir)));
    ApprovalTests::Approvals::verify(actual);
}

TEST(TexttestFixture, SHealthFeatureSnapshotMatchesExpectedFile) {
    const auto expectedFile = std::filesystem::path(SHEALTH_GOLDEN_EXPECTED_DIR) /
                              "SHealthFeatureSnapshot.expected.txt";
    const auto receivedFile = std::filesystem::path(SHEALTH_GOLDEN_EXPECTED_DIR) /
                              "SHealthFeatureSnapshot.received.txt";
    const auto inputFile = writeFeatureGoldenMasterInput();

    ASSERT_TRUE(std::filesystem::exists(expectedFile)) << expectedFile.string();

    const std::string expected = normalizeLineEndings(readFile(expectedFile));
    const std::string actual =
        normalizeLineEndings(buildFeatureSnapshot(inputFile));

    if (expected != actual) {
        writeFile(receivedFile, actual);
    } else if (std::filesystem::exists(receivedFile)) {
        std::filesystem::remove(receivedFile);
    }

    EXPECT_EQ(expected, actual);
}

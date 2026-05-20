#include <gtest/gtest.h>
#include "ApprovalTests.hpp"

#include <cstdio>
#include <filesystem>
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

namespace {
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

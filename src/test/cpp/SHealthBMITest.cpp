#include <gtest/gtest.h>
#include "SHealth.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
constexpr int UnderweightType = 100;
constexpr int NormalType = 200;
constexpr int OverweightType = 300;
constexpr int ObesityType = 400;

class SHealthBMITest : public ::testing::Test {
protected:
    std::filesystem::path writeCsv(const std::string& name,
                                   const std::vector<std::string>& rows) {
        const auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream file(path);
        file << "id,age,weight,height\n";
        for (const auto& row : rows) {
            file << row << '\n';
        }
        return path;
    }

    void expectOnlyRatio(SHealth& health, int ageClass, int expectedType) {
        EXPECT_EQ(expectedType == UnderweightType ? 100.0 : 0.0,
                  health.getBmiRatio(ageClass, UnderweightType));
        EXPECT_EQ(expectedType == NormalType ? 100.0 : 0.0,
                  health.getBmiRatio(ageClass, NormalType));
        EXPECT_EQ(expectedType == OverweightType ? 100.0 : 0.0,
                  health.getBmiRatio(ageClass, OverweightType));
        EXPECT_EQ(expectedType == ObesityType ? 100.0 : 0.0,
                  health.getBmiRatio(ageClass, ObesityType));
    }
};
} // namespace

TEST_F(SHealthBMITest, UnderweightWhenBmiIsBelowLowerBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_underweight_below_boundary.csv",
                               {"1,20,18.4,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 20, UnderweightType);
}

TEST_F(SHealthBMITest, UnderweightWhenBmiEqualsLowerBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_underweight_at_boundary.csv",
                               {"1,29,18.5,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 20, UnderweightType);
}

TEST_F(SHealthBMITest, UnderweightKeepsAgeThirtyBoundaryInThirtyClass) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_underweight_age_thirty.csv",
                               {"1,30,50,170"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 30, UnderweightType);
}

TEST_F(SHealthBMITest, UnderweightKeepsAgeSeventyNineInSeventyClass) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_underweight_age_seventy_nine.csv",
                               {"1,79,45,160"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 70, UnderweightType);
}

TEST_F(SHealthBMITest, UnderweightExcludesNegativeAgeFromRatios) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_underweight_negative_age.csv",
                               {"1,-1,50,170"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    EXPECT_EQ(0.0, health.getBmiRatio(20, UnderweightType));
}

TEST_F(SHealthBMITest, NormalWhenBmiIsJustAboveUnderweightBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_above_underweight.csv",
                               {"1,20,18.6,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 20, NormalType);
}

TEST_F(SHealthBMITest, NormalWhenBmiIsInMiddleOfRange) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_middle.csv",
                               {"1,35,21,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 30, NormalType);
}

TEST_F(SHealthBMITest, NormalWhenBmiIsJustBelowOverweightBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_below_overweight.csv",
                               {"1,49,22.9,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 40, NormalType);
}

TEST_F(SHealthBMITest, NormalUsesImputedWeightFromSameAgeClass) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_imputed_weight.csv",
                               {"1,50,20,100", "2,51,0,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    expectOnlyRatio(health, 50, NormalType);
}

TEST_F(SHealthBMITest, NormalSkipsNegativeWeightRecord) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_negative_weight.csv",
                               {"1,60,-20,100", "2,60,21,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 60, NormalType);
}

TEST_F(SHealthBMITest, OverweightWhenBmiEqualsLowerBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_at_boundary.csv",
                               {"1,20,23,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 20, OverweightType);
}

TEST_F(SHealthBMITest, OverweightWhenBmiIsInMiddleOfRange) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_middle.csv",
                               {"1,31,24,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 30, OverweightType);
}

TEST_F(SHealthBMITest, OverweightWhenBmiIsJustBelowObesityBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_below_obesity.csv",
                               {"1,45,24.9,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 40, OverweightType);
}

TEST_F(SHealthBMITest, OverweightSkipsNegativeHeightRecord) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_negative_height.csv",
                               {"1,50,24,-100", "2,50,24,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 50, OverweightType);
}

TEST_F(SHealthBMITest, OverweightSkipsZeroHeightRecord) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_zero_height.csv",
                               {"1,60,24,0", "2,60,24,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 60, OverweightType);
}

TEST_F(SHealthBMITest, ObesityWhenBmiEqualsLowerBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_obesity_at_boundary.csv",
                               {"1,20,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 20, ObesityType);
}

TEST_F(SHealthBMITest, ObesityWhenBmiIsAboveLowerBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_obesity_above_boundary.csv",
                               {"1,32,30,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 30, ObesityType);
}

TEST_F(SHealthBMITest, ObesityWhenHeightIsTwoMetersAndWeightIsOneHundred) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_obesity_two_meter_boundary.csv",
                               {"1,44,100,200"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 40, ObesityType);
}

TEST_F(SHealthBMITest, ObesityKeepsAgeSeventyBoundaryInSeventyClass) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_obesity_age_seventy.csv",
                               {"1,70,26,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    expectOnlyRatio(health, 70, ObesityType);
}

TEST_F(SHealthBMITest, ObesityExcludesAgeEightyFromRatios) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_obesity_age_eighty.csv",
                               {"1,80,26,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    EXPECT_EQ(0.0, health.getBmiRatio(70, ObesityType));
}

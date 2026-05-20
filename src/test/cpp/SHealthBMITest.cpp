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

TEST_F(SHealthBMITest, OverweightImputesZeroHeightRecord) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overweight_zero_height.csv",
                               {"1,60,24,0", "2,60,24,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
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

TEST_F(SHealthBMITest, AgeDistributionCalculatesAllFourCategoryRatiosForOneAgeClass) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_age_distribution_all_categories.csv",
                               {"1,20,18.5,100", "2,21,20,100",
                                "3,22,23,100", "4,23,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(4, recordCount);
    EXPECT_EQ(25.0, health.getBmiRatio(20, UnderweightType));
    EXPECT_EQ(25.0, health.getBmiRatio(20, NormalType));
    EXPECT_EQ(25.0, health.getBmiRatio(20, OverweightType));
    EXPECT_EQ(25.0, health.getBmiRatio(20, ObesityType));
}

TEST_F(SHealthBMITest, AgeDistributionGroupsAgesTwentyAndTwentyNineTogether) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_age_distribution_twenty_boundaries.csv",
                               {"1,20,18.5,100", "2,29,20,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(50.0, health.getBmiRatio(20, UnderweightType));
    EXPECT_EQ(50.0, health.getBmiRatio(20, NormalType));
}

TEST_F(SHealthBMITest, AgeDistributionExcludesAgeBelowTwentyFromAgeClassRatio) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_age_distribution_below_twenty.csv",
                               {"1,19,18.5,100", "2,20,20,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(0.0, health.getBmiRatio(20, UnderweightType));
    EXPECT_EQ(100.0, health.getBmiRatio(20, NormalType));
}

TEST_F(SHealthBMITest, AgeDistributionExcludesAgeEightyFromSeventyClassRatio) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_age_distribution_age_eighty.csv",
                               {"1,79,23,100", "2,80,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(100.0, health.getBmiRatio(70, OverweightType));
    EXPECT_EQ(0.0, health.getBmiRatio(70, ObesityType));
}

TEST_F(SHealthBMITest, AgeDistributionReturnsZeroForAgeClassWithNoValidBmi) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_age_distribution_empty_class.csv",
                               {"1,30,20,0", "2,31,0,0"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(0.0, health.getBmiRatio(30, UnderweightType));
    EXPECT_EQ(0.0, health.getBmiRatio(30, NormalType));
    EXPECT_EQ(0.0, health.getBmiRatio(30, OverweightType));
    EXPECT_EQ(0.0, health.getBmiRatio(30, ObesityType));
}

TEST_F(SHealthBMITest, HeightZeroImputesSameAgeClassAverageHeight) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_height_zero_imputes_average.csv",
                               {"1,20,72,180", "2,21,72,0"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(100.0, health.getBmiRatio(20, NormalType));
}

TEST_F(SHealthBMITest, HeightZeroUsesAverageOfMultipleSameAgeClassHeights) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_height_zero_uses_average.csv",
                               {"1,30,45,160", "2,31,81,180",
                                "3,32,60,0"});

    // When
    const int recordCount = health.calculateBmi(path.string());
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(3, recordCount);
    ASSERT_EQ(1U, normalUsers.size());
    EXPECT_EQ("3", normalUsers[0].id);
    EXPECT_EQ(170.0, normalUsers[0].height);
}

TEST_F(SHealthBMITest, HeightZeroWithoutAverageIsExcludedFromRatios) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_height_zero_without_average.csv",
                               {"1,40,60,0", "2,41,70,0"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(0.0, health.getBmiRatio(40, UnderweightType));
    EXPECT_EQ(0.0, health.getBmiRatio(40, NormalType));
    EXPECT_EQ(0.0, health.getBmiRatio(40, OverweightType));
    EXPECT_EQ(0.0, health.getBmiRatio(40, ObesityType));
}

TEST_F(SHealthBMITest, HeightNegativeRecordIsSkippedBeforeImputation) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_height_negative_skipped.csv",
                               {"1,50,60,-170", "2,50,60,170"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    EXPECT_EQ(100.0, health.getBmiRatio(50, NormalType));
}

TEST_F(SHealthBMITest, HeightZeroUsesTwentyAgeClassAtAgeTwentyNineBoundary) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_height_zero_age_boundary.csv",
                               {"1,20,72,180", "2,29,72,0",
                                "3,30,100,200"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(3, recordCount);
    EXPECT_EQ(100.0, health.getBmiRatio(20, NormalType));
    EXPECT_EQ(100.0, health.getBmiRatio(30, ObesityType));
}

TEST_F(SHealthBMITest, NormalBmiUsersReturnsOnlyNormalCategoryUsers) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_users_only_normal.csv",
                               {"under,20,18.5,100", "normal,20,20,100",
                                "over,20,23,100", "obese,20,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(4, recordCount);
    ASSERT_EQ(1U, normalUsers.size());
    EXPECT_EQ("normal", normalUsers[0].id);
}

TEST_F(SHealthBMITest, NormalBmiUsersExcludesExactBoundaryValues) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_users_excludes_boundaries.csv",
                               {"low,20,18.5,100", "inside,20,18.6,100",
                                "high,20,23,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(3, recordCount);
    ASSERT_EQ(1U, normalUsers.size());
    EXPECT_EQ("inside", normalUsers[0].id);
}

TEST_F(SHealthBMITest, NormalBmiUsersPreservesCsvUserIds) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_users_preserves_ids.csv",
                               {"user-001,35,20,100", "user-002,35,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(2, recordCount);
    ASSERT_EQ(1U, normalUsers.size());
    EXPECT_EQ("user-001", normalUsers[0].id);
    EXPECT_EQ(35, normalUsers[0].age);
}

TEST_F(SHealthBMITest, NormalBmiUsersPreservesInputOrder) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_normal_users_preserves_order.csv",
                               {"first,40,20,100", "skip,40,25,100",
                                "second,40,22,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(3, recordCount);
    ASSERT_EQ(2U, normalUsers.size());
    EXPECT_EQ("first", normalUsers[0].id);
    EXPECT_EQ("second", normalUsers[1].id);
}

TEST_F(SHealthBMITest, NormalBmiUsersIsEmptyAfterMissingFile) {
    // Given
    SHealth health;

    // When
    const int recordCount = health.calculateBmi("missing_normal_bmi_users.csv");
    const auto normalUsers = health.getNormalBmiUsers();

    // Then
    ASSERT_EQ(0, recordCount);
    EXPECT_EQ(0U, normalUsers.size());
}

TEST_F(SHealthBMITest, OverallBmiRatioCalculatesAllFourCategories) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overall_all_categories.csv",
                               {"1,20,18.5,100", "2,30,20,100",
                                "3,40,23,100", "4,50,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(4, recordCount);
    EXPECT_EQ(25.0, health.getOverallBmiRatio(UnderweightType));
    EXPECT_EQ(25.0, health.getOverallBmiRatio(NormalType));
    EXPECT_EQ(25.0, health.getOverallBmiRatio(OverweightType));
    EXPECT_EQ(25.0, health.getOverallBmiRatio(ObesityType));
}

TEST_F(SHealthBMITest, OverallBmiRatioIncludesAgesOutsideAgeClassRange) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overall_includes_outside_ages.csv",
                               {"1,19,20,100", "2,80,25,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(50.0, health.getOverallBmiRatio(NormalType));
    EXPECT_EQ(50.0, health.getOverallBmiRatio(ObesityType));
    EXPECT_EQ(0.0, health.getBmiRatio(20, NormalType));
    EXPECT_EQ(0.0, health.getBmiRatio(70, ObesityType));
}

TEST_F(SHealthBMITest, OverallBmiRatioExcludesRecordsWithUnresolvedZeroHeight) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overall_excludes_unresolved_height.csv",
                               {"1,20,60,0", "2,30,20,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(2, recordCount);
    EXPECT_EQ(100.0, health.getOverallBmiRatio(NormalType));
    EXPECT_EQ(0.0, health.getOverallBmiRatio(UnderweightType));
}

TEST_F(SHealthBMITest, OverallBmiRatioReturnsZeroForInvalidType) {
    // Given
    SHealth health;
    const auto path = writeCsv("shealth_overall_invalid_type.csv",
                               {"1,20,20,100"});

    // When
    const int recordCount = health.calculateBmi(path.string());

    // Then
    ASSERT_EQ(1, recordCount);
    EXPECT_EQ(0.0, health.getOverallBmiRatio(999));
}

TEST_F(SHealthBMITest, OverallBmiRatioResetsBetweenCalculateCalls) {
    // Given
    SHealth health;
    const auto firstPath = writeCsv("shealth_overall_reset_first.csv",
                                    {"1,20,25,100"});
    const auto secondPath = writeCsv("shealth_overall_reset_second.csv",
                                     {"2,20,20,100"});

    // When
    const int firstRecordCount = health.calculateBmi(firstPath.string());
    const int secondRecordCount = health.calculateBmi(secondPath.string());

    // Then
    ASSERT_EQ(1, firstRecordCount);
    ASSERT_EQ(1, secondRecordCount);
    EXPECT_EQ(0.0, health.getOverallBmiRatio(ObesityType));
    EXPECT_EQ(100.0, health.getOverallBmiRatio(NormalType));
}

#include "StandardBmiClassifier.h"

namespace {
constexpr double UnderweightMax = 18.5;
constexpr double NormalMax = 23.0;
constexpr double OverweightMax = 25.0;
} // namespace

BmiCategory StandardBmiClassifier::classify(double bmi) const {
    if (bmi <= UnderweightMax) {
        return BmiCategory::Underweight;
    }
    if (bmi < NormalMax) {
        return BmiCategory::Normal;
    }
    return bmi < OverweightMax ? BmiCategory::Overweight : BmiCategory::Obesity;
}

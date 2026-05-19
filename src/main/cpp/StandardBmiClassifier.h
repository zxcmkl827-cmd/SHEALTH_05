#pragma once

#include "BmiClassifier.h"

class StandardBmiClassifier : public BmiClassifier {
public:
    BmiCategory classify(double bmi) const override;
};

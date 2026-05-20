#pragma once

#include "BmiCategory.h"

class BmiClassifier {
public:
    virtual ~BmiClassifier() = default;
    virtual BmiCategory classify(double bmi) const = 0;
};

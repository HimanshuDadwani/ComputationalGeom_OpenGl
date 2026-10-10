#pragma once

#include "BezierCurve.h"
#include <vector>

struct BezierIntersectionResult
{
    Point point;
    double firstParameter = 0.0;
    double secondParameter = 0.0;
};

struct BezierIntersectionOptions
{
    double positionTolerance = 1e-6;
    double parameterTolerance = 1e-8;
    int maxSubdivisionDepth = 20;
    int maxNewtonIterations = 30;
    int newtonSeedCount = 12;
};

class BezierIntersection
{
public:
    static std::vector<BezierIntersectionResult> SolveSubdivision(
        const BezierCurve& first,
        const BezierCurve& second,
        const BezierIntersectionOptions& options = {});

    static std::vector<BezierIntersectionResult> SolvePolynomial(
        const BezierCurve& first,
        const BezierCurve& second,
        const BezierIntersectionOptions& options = {});

    // Returns true when both solvers find the expected crossing.
    static bool TestIntersection();
};

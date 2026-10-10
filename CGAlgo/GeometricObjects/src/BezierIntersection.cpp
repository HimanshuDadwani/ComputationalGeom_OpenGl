#include "BezierIntersection.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
struct ParameterBox
{
    double firstMin;
    double firstMax;
    double secondMin;
    double secondMax;
};

struct Bounds
{
    double minX = std::numeric_limits<double>::infinity();
    double maxX = -std::numeric_limits<double>::infinity();
    double minY = std::numeric_limits<double>::infinity();
    double maxY = -std::numeric_limits<double>::infinity();
};

bool IsFinite(const Point& point)
{
    return std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.z);
}

double DistanceSquaredXY(const Point& first, const Point& second)
{
    const double dx = first.x - second.x;
    const double dy = first.y - second.y;
    return dx * dx + dy * dy;
}

Bounds GetBounds(const BezierCurve& curve, double minParameter, double maxParameter)
{
    Bounds bounds;
    const double samples[] = {
        minParameter,
        minParameter + (maxParameter - minParameter) * 0.25,
        minParameter + (maxParameter - minParameter) * 0.5,
        minParameter + (maxParameter - minParameter) * 0.75,
        maxParameter
    };

    for (double parameter : samples)
    {
        const Point point = curve.EvaluateAt(parameter);
        bounds.minX = std::min(bounds.minX, point.x);
        bounds.maxX = std::max(bounds.maxX, point.x);
        bounds.minY = std::min(bounds.minY, point.y);
        bounds.maxY = std::max(bounds.maxY, point.y);
    }
    return bounds;
}

bool Overlap(const Bounds& first, const Bounds& second, double tolerance)
{
    return first.minX <= second.maxX + tolerance &&
           second.minX <= first.maxX + tolerance &&
           first.minY <= second.maxY + tolerance &&
           second.minY <= first.maxY + tolerance;
}

void AddResult(
    std::vector<BezierIntersectionResult>& results,
    const BezierCurve& first,
    const BezierCurve& second,
    double firstParameter,
    double secondParameter,
    const BezierIntersectionOptions& options)
{
    firstParameter = std::max(0.0, std::min(1.0, firstParameter));
    secondParameter = std::max(0.0, std::min(1.0, secondParameter));

    const Point firstPoint = first.EvaluateAt(firstParameter);
    const Point secondPoint = second.EvaluateAt(secondParameter);
    if (!IsFinite(firstPoint) || !IsFinite(secondPoint) ||
        DistanceSquaredXY(firstPoint, secondPoint) > options.positionTolerance * options.positionTolerance)
    {
        return;
    }

    for (const BezierIntersectionResult& result : results)
    {
        if (std::abs(result.firstParameter - firstParameter) <= options.parameterTolerance &&
            std::abs(result.secondParameter - secondParameter) <= options.parameterTolerance)
        {
            return;
        }
    }

    BezierIntersectionResult result;
    result.point = Point(
        (firstPoint.x + secondPoint.x) * 0.5,
        (firstPoint.y + secondPoint.y) * 0.5,
        (firstPoint.z + secondPoint.z) * 0.5);
    result.firstParameter = firstParameter;
    result.secondParameter = secondParameter;
    results.push_back(result);
}

bool RefineNewton(
    const BezierCurve& first,
    const BezierCurve& second,
    double& firstParameter,
    double& secondParameter,
    const BezierIntersectionOptions& options)
{
    for (int iteration = 0; iteration < options.maxNewtonIterations; ++iteration)
    {
        const Point firstPoint = first.EvaluateAt(firstParameter);
        const Point secondPoint = second.EvaluateAt(secondParameter);
        const Vector firstDerivative = first.FirstDerivative(firstParameter);
        const Vector secondDerivative = second.FirstDerivative(secondParameter);
        const double errorX = firstPoint.x - secondPoint.x;
        const double errorY = firstPoint.y - secondPoint.y;
        const double errorSquared = errorX * errorX + errorY * errorY;

        if (errorSquared <= options.positionTolerance * options.positionTolerance)
        {
            return true;
        }

        const double determinant = firstDerivative._y * secondDerivative._x -
                                   firstDerivative._x * secondDerivative._y;
        if (std::abs(determinant) <= std::numeric_limits<double>::epsilon())
        {
            return false;
        }

        const double firstStep = (errorX * secondDerivative._y - errorY * secondDerivative._x) / determinant;
        const double secondStep = (firstDerivative._x * errorY - firstDerivative._y * errorX) / determinant;
        firstParameter -= firstStep;
        secondParameter -= secondStep;

        if (firstParameter < -options.parameterTolerance || firstParameter > 1.0 + options.parameterTolerance ||
            secondParameter < -options.parameterTolerance || secondParameter > 1.0 + options.parameterTolerance)
        {
            return false;
        }

        firstParameter = std::max(0.0, std::min(1.0, firstParameter));
        secondParameter = std::max(0.0, std::min(1.0, secondParameter));
    }

    return DistanceSquaredXY(first.EvaluateAt(firstParameter), second.EvaluateAt(secondParameter)) <=
           options.positionTolerance * options.positionTolerance;
}

void Subdivide(
    const BezierCurve& first,
    const BezierCurve& second,
    const ParameterBox& box,
    int depth,
    const BezierIntersectionOptions& options,
    std::vector<BezierIntersectionResult>& results)
{
    const Bounds firstBounds = GetBounds(first, box.firstMin, box.firstMax);
    const Bounds secondBounds = GetBounds(second, box.secondMin, box.secondMax);
    if (!Overlap(firstBounds, secondBounds, options.positionTolerance))
    {
        return;
    }

    const double firstSize = box.firstMax - box.firstMin;
    const double secondSize = box.secondMax - box.secondMin;
    if (depth >= options.maxSubdivisionDepth ||
        (firstSize <= options.parameterTolerance && secondSize <= options.parameterTolerance))
    {
        double firstParameter = (box.firstMin + box.firstMax) * 0.5;
        double secondParameter = (box.secondMin + box.secondMax) * 0.5;
        if (RefineNewton(first, second, firstParameter, secondParameter, options))
        {
            AddResult(results, first, second, firstParameter, secondParameter, options);
        }
        return;
    }

    if (firstSize >= secondSize)
    {
        const double middle = (box.firstMin + box.firstMax) * 0.5;
        Subdivide(first, second, { box.firstMin, middle, box.secondMin, box.secondMax }, depth + 1, options, results);
        Subdivide(first, second, { middle, box.firstMax, box.secondMin, box.secondMax }, depth + 1, options, results);
    }
    else
    {
        const double middle = (box.secondMin + box.secondMax) * 0.5;
        Subdivide(first, second, { box.firstMin, box.firstMax, box.secondMin, middle }, depth + 1, options, results);
        Subdivide(first, second, { box.firstMin, box.firstMax, middle, box.secondMax }, depth + 1, options, results);
    }
}

void SortResults(std::vector<BezierIntersectionResult>& results)
{
    std::sort(results.begin(), results.end(), [](const BezierIntersectionResult& first, const BezierIntersectionResult& second)
    {
        if (first.firstParameter != second.firstParameter)
        {
            return first.firstParameter < second.firstParameter;
        }
        return first.secondParameter < second.secondParameter;
    });
}
}

std::vector<BezierIntersectionResult> BezierIntersection::SolveSubdivision(
    const BezierCurve& first,
    const BezierCurve& second,
    const BezierIntersectionOptions& options)
{
    std::vector<BezierIntersectionResult> results;
    if (first.GetNumControlPoints() == 0 || second.GetNumControlPoints() == 0)
    {
        return results;
    }

    Subdivide(first, second, { 0.0, 1.0, 0.0, 1.0 }, 0, options, results);
    SortResults(results);
    return results;
}

std::vector<BezierIntersectionResult> BezierIntersection::SolvePolynomial(
    const BezierCurve& first,
    const BezierCurve& second,
    const BezierIntersectionOptions& options)
{
    std::vector<BezierIntersectionResult> results;
    if (first.GetNumControlPoints() == 0 || second.GetNumControlPoints() == 0)
    {
        return results;
    }

    const int seedCount = std::max(2, options.newtonSeedCount);
    for (int firstSeed = 0; firstSeed < seedCount; ++firstSeed)
    {
        for (int secondSeed = 0; secondSeed < seedCount; ++secondSeed)
        {
            double firstParameter = static_cast<double>(firstSeed) / (seedCount - 1);
            double secondParameter = static_cast<double>(secondSeed) / (seedCount - 1);
            if (RefineNewton(first, second, firstParameter, secondParameter, options))
            {
                AddResult(results, first, second, firstParameter, secondParameter, options);
            }
        }
    }

    SortResults(results);
    return results;
}

bool BezierIntersection::TestIntersection()
{
    BezierCurve first;
    BezierCurve second;
    std::vector<Point> firstControlPoints{
        Point(0.0, 0.0), Point(0.5, 1.0), Point(1.0, 0.0) };
    std::vector<Point> secondControlPoints{
        Point(0.0, 1.0), Point(0.5, 0.0), Point(1.0, 1.0) };
    first.SetControlPoints(firstControlPoints);
    second.SetControlPoints(secondControlPoints);

    const BezierIntersectionOptions options;
    const std::vector<BezierIntersectionResult> subdivisionResults =
        SolveSubdivision(first, second, options);
    const std::vector<BezierIntersectionResult> polynomialResults =
        SolvePolynomial(first, second, options);

    auto HasExpectedCrossing = [&first, &second, &options](
        const std::vector<BezierIntersectionResult>& results)
    {
        for (const BezierIntersectionResult& result : results)
        {
            const Point firstPoint = first.EvaluateAt(result.firstParameter);
            const Point secondPoint = second.EvaluateAt(result.secondParameter);
            const double dx = firstPoint.x - secondPoint.x;
            const double dy = firstPoint.y - secondPoint.y;
            if (result.firstParameter >= 0.0 && result.firstParameter <= 1.0 &&
                result.secondParameter >= 0.0 && result.secondParameter <= 1.0 &&
                std::abs(result.point.x - 0.5) <= options.positionTolerance &&
                std::abs(result.point.y - 0.5) <= options.positionTolerance &&
                dx * dx + dy * dy <= options.positionTolerance * options.positionTolerance)
            {
                return true;
            }
        }
        return false;
    };

    const bool passed = HasExpectedCrossing(subdivisionResults) &&
                        HasExpectedCrossing(polynomialResults);
    std::cout << "BezierIntersection::TestIntersection: "
              << (passed ? "passed" : "failed") << std::endl;
    return passed;
}

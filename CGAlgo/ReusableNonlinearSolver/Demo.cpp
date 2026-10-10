#include <iostream>
#include <vector>

#include "GeometryOperations.h"
#include "MathProblem.h"
#include "SolverStrategies.h"

int main()
{
    using namespace CGAlgo;

    // --- 1D scalar root test ---
    Math::ScalarRootProblem scalarProblem(
        [](double x) { return x * x - 4.0; },
        [](double x) { return 2.0 * x; },
        3.0,
        1e-10,
        50);

    Math::NewtonSolverStrategy scalarNewton;
    const Math::SolverResult scalarResult = scalarNewton.Solve(scalarProblem);
    std::cout << "Scalar root result: converged=" << scalarResult.converged << " x=" << scalarResult.x[0] << std::endl;

    // --- 2D nonlinear system test ---
    Math::NonlinearSystemProblem systemProblem(
        { 1.0, 1.0 },
        [](const std::vector<double>& x) {
            return std::vector<double>{ x[0] * x[0] + x[1] * x[1] - 4.0, x[0] - x[1] };
        },
        [](const std::vector<double>& x) {
            return std::vector<std::vector<double>>{{ 2.0 * x[0], 2.0 * x[1] }, { 1.0, -1.0 }};
        },
        1e-10,
        50);

    Math::GaussNewtonSolverStrategy gn;
    const Math::SolverResult systemResult = gn.Solve(systemProblem);
    std::cout << "System result: converged=" << systemResult.converged << " x=" << systemResult.x[0] << ", " << systemResult.x[1] << std::endl;

    // --- Bezier curve intersection example using a simplified case ---
    Math::BezierCurve2D curveA({ {0.0, 0.0}, {1.0, 2.0}, {2.0, 0.0} });
    Math::BezierCurve2D curveB({ {0.0, 2.0}, {1.0, 1.0}, {2.0, 0.0} });

    auto debugVisualizer = std::make_shared<Debug::MathDebugVisualizer>();
    Geometry::BezierCurveIntersectionOperation intersection(curveA, curveB, { 0.3, 0.7 }, 1e-8, 50);
    intersection.SetDebugVisualizer(debugVisualizer);

    Math::GradientDescentSolverStrategy descent;
    const Math::SolverResult intersectionResult = intersection.Solve(descent);
    std::cout << "Intersection result: converged=" << intersectionResult.converged << " params=" << intersectionResult.x[0] << ", " << intersectionResult.x[1] << std::endl;

    // --- Curve fitting example ---
    std::vector<Math::CurveFitSample> samples = {
        { -1.0, 1.0 },
        { 0.0, 0.0 },
        { 1.0, 1.0 },
        { 2.0, 4.0 }
    };

    auto quadraticFitModel = [](const std::vector<double>& params, double x) {
        return params[0] * x * x + params[1] * x + params[2];
    };

    Geometry::CurveFittingOperation fit({ 1.0, 0.0, 0.0 }, samples, quadraticFitModel, 1e-10, 50);
    fit.SetDebugVisualizer(std::make_shared<Debug::MathDebugVisualizer>());
    Math::NewtonSolverStrategy fitNewton;
    const Math::SolverResult fitResult = fit.Solve(fitNewton);
    std::cout << "Fit result: converged=" << fitResult.converged << " params=" << fitResult.x[0] << ", " << fitResult.x[1] << ", " << fitResult.x[2] << std::endl;

    // --- Curve fairing example ---
    std::vector<Math::Vec2> points = { {0.0, 0.0}, {1.0, 2.0}, {2.0, 1.5}, {3.0, 0.0}, {4.0, 1.0} };
    Geometry::CurveFairingOperation fairing(points, 1e-8, 80);
    fairing.SetDebugVisualizer(std::make_shared<Debug::MathDebugVisualizer>());
    Math::GradientDescentSolverStrategy fairingStrategy;
    const Math::SolverResult fairingResult = fairing.Solve(fairingStrategy);
    std::cout << "Fairing result: converged=" << fairingResult.converged << " iterations=" << fairingResult.iterations << std::endl;

    return 0;
}

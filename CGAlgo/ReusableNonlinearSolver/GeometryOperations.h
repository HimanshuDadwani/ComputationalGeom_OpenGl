#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "DebugVisualizer.h"
#include "MathProblem.h"
#include "SolverStrategies.h"

namespace CGAlgo {
namespace Geometry {

class GeometryOperation
{
public:
    virtual ~GeometryOperation() = default;

    virtual Math::SolverResult Solve(Math::MathSolverStrategy& strategy) = 0;
    virtual void SetDebugVisualizer(std::shared_ptr<Debug::MathDebugVisualizer> visualizer)
    {
        m_visualizer = std::move(visualizer);
    }

protected:
    std::shared_ptr<Debug::MathDebugVisualizer> m_visualizer;
};

class BezierCurveIntersectionOperation : public GeometryOperation
{
public:
    BezierCurveIntersectionOperation(
        Math::BezierCurve2D curveA,
        Math::BezierCurve2D curveB,
        std::vector<double> initialGuess,
        double tolerance = 1e-8,
        int maxIterations = 50)
        : m_curveA(std::move(curveA)), m_curveB(std::move(curveB)), m_initialGuess(std::move(initialGuess)), m_tolerance(tolerance), m_maxIterations(maxIterations)
    {
        m_problem = std::make_unique<Math::BezierIntersectionProblem>(m_curveA, m_curveB, m_initialGuess, m_tolerance, m_maxIterations);
    }

    Math::SolverResult Solve(Math::MathSolverStrategy& strategy) override
    {
        if (m_visualizer)
            m_visualizer->RecordIteration("Bezier intersection start", m_initialGuess, { 0.0, 0.0 });

        const Math::SolverResult result = strategy.Solve(*m_problem);

        if (m_visualizer)
        {
            const Math::Vec2 pa = m_curveA.Evaluate(result.x[0]);
            const Math::Vec2 pb = m_curveB.Evaluate(result.x[1]);
            m_visualizer->RecordPoint("CurveA root", pa);
            m_visualizer->RecordPoint("CurveB root", pb);
            m_visualizer->RecordIteration("Bezier intersection end", result.x, m_problem->Residual(result.x));
        }

        return result;
    }

private:
    Math::BezierCurve2D m_curveA;
    Math::BezierCurve2D m_curveB;
    std::vector<double> m_initialGuess;
    double m_tolerance = 1e-8;
    int m_maxIterations = 50;
    std::unique_ptr<Math::BezierIntersectionProblem> m_problem;
};

class CurveFittingOperation : public GeometryOperation
{
public:
    CurveFittingOperation(
        std::vector<double> initialParams,
        std::vector<Math::CurveFitSample> samples,
        std::function<double(const std::vector<double>&, double)> model,
        double tolerance = 1e-8,
        int maxIterations = 50)
        : m_initialParams(std::move(initialParams)), m_samples(std::move(samples)), m_model(std::move(model)), m_tolerance(tolerance), m_maxIterations(maxIterations)
    {
        m_problem = std::make_unique<Math::CurveFittingProblem>(m_initialParams, m_samples, m_model, m_tolerance, m_maxIterations);
    }

    Math::SolverResult Solve(Math::MathSolverStrategy& strategy) override
    {
        if (m_visualizer)
            m_visualizer->RecordIteration("Curve fit start", m_initialParams, m_problem->Residual(m_initialParams));

        const Math::SolverResult result = strategy.Solve(*m_problem);

        if (m_visualizer)
        {
            m_visualizer->RecordIteration("Curve fit end", result.x, m_problem->Residual(result.x));
            for (const auto& sample : m_samples)
            {
                const double predicted = m_model(result.x, sample.x);
                m_visualizer->RecordPoint("Sample", { sample.x, predicted });
            }
        }

        return result;
    }

private:
    std::vector<double> m_initialParams;
    std::vector<Math::CurveFitSample> m_samples;
    std::function<double(const std::vector<double>&, double)> m_model;
    double m_tolerance = 1e-8;
    int m_maxIterations = 50;
    std::unique_ptr<Math::CurveFittingProblem> m_problem;
};

class CurveFairingOperation : public GeometryOperation
{
public:
    CurveFairingOperation(
        std::vector<Math::Vec2> controlPoints,
        double tolerance = 1e-8,
        int maxIterations = 50)
        : m_points(std::move(controlPoints)), m_tolerance(tolerance), m_maxIterations(maxIterations)
    {
        m_problem = std::make_unique<Math::CurveFairingProblem>(m_points, m_tolerance, m_maxIterations);
    }

    Math::SolverResult Solve(Math::MathSolverStrategy& strategy) override
    {
        if (m_visualizer)
            m_visualizer->RecordIteration("Fairing start", m_problem->InitialGuess(), m_problem->Residual(m_problem->InitialGuess()));

        const Math::SolverResult result = strategy.Solve(*m_problem);

        if (m_visualizer)
        {
            m_visualizer->RecordIteration("Fairing end", result.x, m_problem->Residual(result.x));
            for (std::size_t i = 0; i + 1 < result.x.size(); i += 2)
                m_visualizer->RecordPoint("Fair point", { result.x[i], result.x[i + 1] });
        }

        return result;
    }

private:
    std::vector<Math::Vec2> m_points;
    double m_tolerance = 1e-8;
    int m_maxIterations = 50;
    std::unique_ptr<Math::CurveFairingProblem> m_problem;
};

} // namespace Geometry
} // namespace CGAlgo

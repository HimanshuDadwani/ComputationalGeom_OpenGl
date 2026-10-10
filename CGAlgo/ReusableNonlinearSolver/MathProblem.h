#pragma once

#include <cmath>
#include <functional>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace CGAlgo {
namespace Math {

struct Vec2
{
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double xIn, double yIn) : x(xIn), y(yIn) {}
};

struct SolverResult
{
    bool converged = false;
    int iterations = 0;
    std::vector<double> x;
    double residualNorm = std::numeric_limits<double>::infinity();
    std::string message;
};

class MathProblem
{
public:
    virtual ~MathProblem() = default;

    virtual std::string Name() const = 0;
    virtual bool IsValid() const = 0;
    virtual void Reset() = 0;

    virtual std::vector<double> Residual(const std::vector<double>& x) const = 0;
    virtual std::vector<std::vector<double>> Jacobian(const std::vector<double>& x) const
    {
        (void)x;
        return {};
    }

    virtual std::vector<double> InitialGuess() const = 0;
    virtual std::vector<double> CurrentState() const = 0;
    virtual void SetState(const std::vector<double>& value) = 0;
};

class ScalarRootProblem : public MathProblem
{
public:
    double x = 0.0;
    std::function<double(double)> residual;
    std::function<double(double)> derivative;
    double tolerance = 1e-8;
    int maxIterations = 50;

    ScalarRootProblem() = default;

    ScalarRootProblem(
        std::function<double(double)> residualFunction,
        std::function<double(double)> derivativeFunction,
        double initialGuess,
        double tol = 1e-8,
        int maxItr = 50)
        : x(initialGuess), residual(std::move(residualFunction)), derivative(std::move(derivativeFunction)), tolerance(tol), maxIterations(maxItr)
    {
    }

    std::string Name() const override { return "ScalarRootProblem"; }

    bool IsValid() const override
    {
        return residual != nullptr && derivative != nullptr;
    }

    void Reset() override
    {
        x = 0.0;
    }

    std::vector<double> Residual(const std::vector<double>& value) const override
    {
        return { residual(value[0]) };
    }

    std::vector<std::vector<double>> Jacobian(const std::vector<double>& value) const override
    {
        return { { derivative(value[0]) } };
    }

    std::vector<double> InitialGuess() const override
    {
        return { x };
    }

    std::vector<double> CurrentState() const override
    {
        return { x };
    }

    void SetState(const std::vector<double>& value) override
    {
        if (value.empty())
            return;
        x = value[0];
    }
};

class NonlinearSystemProblem : public MathProblem
{
public:
    std::vector<double> x;
    std::function<std::vector<double>(const std::vector<double>&)> residual;
    std::function<std::vector<std::vector<double>>(const std::vector<double>&)> jacobian;
    double tolerance = 1e-8;
    int maxIterations = 50;

    NonlinearSystemProblem() = default;

    NonlinearSystemProblem(
        std::vector<double> initialGuess,
        std::function<std::vector<double>(const std::vector<double>&)> residualFunction,
        std::function<std::vector<std::vector<double>>(const std::vector<double>&)> jacobianFunction,
        double tol = 1e-8,
        int maxItr = 50)
        : x(std::move(initialGuess)), residual(std::move(residualFunction)), jacobian(std::move(jacobianFunction)), tolerance(tol), maxIterations(maxItr)
    {
    }

    std::string Name() const override { return "NonlinearSystemProblem"; }

    bool IsValid() const override
    {
        return residual != nullptr && !x.empty();
    }

    void Reset() override
    {
        x.clear();
    }

    std::vector<double> Residual(const std::vector<double>& value) const override
    {
        if (residual == nullptr)
            return {};
        return residual(value);
    }

    std::vector<std::vector<double>> Jacobian(const std::vector<double>& value) const override
    {
        if (jacobian == nullptr)
            return {};
        return jacobian(value);
    }

    std::vector<double> InitialGuess() const override
    {
        return x;
    }

    std::vector<double> CurrentState() const override
    {
        return x;
    }

    void SetState(const std::vector<double>& value) override
    {
        x = value;
    }
};

struct CurveFitSample
{
    double x = 0.0;
    double y = 0.0;
};

class CurveFittingProblem : public MathProblem
{
public:
    std::vector<double> parameters;
    std::vector<CurveFitSample> samples;
    std::function<double(const std::vector<double>&, double)> model;
    double tolerance = 1e-8;
    int maxIterations = 50;

    CurveFittingProblem() = default;

    CurveFittingProblem(
        std::vector<double> initialParams,
        std::vector<CurveFitSample> data,
        std::function<double(const std::vector<double>&, double)> fitModel,
        double tol = 1e-8,
        int maxItr = 50)
        : parameters(std::move(initialParams)), samples(std::move(data)), model(std::move(fitModel)), tolerance(tol), maxIterations(maxItr)
    {
    }

    std::string Name() const override { return "CurveFittingProblem"; }

    bool IsValid() const override
    {
        return model != nullptr && !samples.empty() && !parameters.empty();
    }

    void Reset() override
    {
        parameters.clear();
    }

    std::vector<double> Residual(const std::vector<double>& value) const override
    {
        std::vector<double> residual;
        residual.reserve(samples.size());

        for (const auto& sample : samples)
        {
            const double predicted = model(value, sample.x);
            residual.push_back(predicted - sample.y);
        }

        return residual;
    }

    std::vector<std::vector<double>> Jacobian(const std::vector<double>& value) const override
    {
        const int paramCount = static_cast<int>(parameters.size());
        const int sampleCount = static_cast<int>(samples.size());
        std::vector<std::vector<double>> jacobian(sampleCount, std::vector<double>(paramCount, 0.0));

        const double eps = 1e-6;
        for (int j = 0; j < paramCount; ++j)
        {
            std::vector<double> plus = value;
            plus[j] += eps;

            std::vector<double> minus = value;
            minus[j] -= eps;

            const double fPlus = model(plus, samples[0].x);
            const double fMinus = model(minus, samples[0].x);
            (void)fPlus;
            (void)fMinus;

            for (int i = 0; i < sampleCount; ++i)
            {
                const double yPlus = model(plus, samples[i].x);
                const double yMinus = model(minus, samples[i].x);
                jacobian[i][j] = (yPlus - yMinus) / (2.0 * eps);
            }
        }

        return jacobian;
    }

    std::vector<double> InitialGuess() const override
    {
        return parameters;
    }

    std::vector<double> CurrentState() const override
    {
        return parameters;
    }

    void SetState(const std::vector<double>& value) override
    {
        parameters = value;
    }
};

class CurveFairingProblem : public MathProblem
{
public:
    std::vector<Vec2> controlPoints;
    double tolerance = 1e-8;
    int maxIterations = 50;

    CurveFairingProblem() = default;

    CurveFairingProblem(std::vector<Vec2> points, double tol = 1e-8, int maxItr = 50)
        : controlPoints(std::move(points)), tolerance(tol), maxIterations(maxItr)
    {
    }

    std::string Name() const override { return "CurveFairingProblem"; }

    bool IsValid() const override
    {
        return controlPoints.size() >= 3;
    }

    void Reset() override
    {
        controlPoints.clear();
    }

    std::vector<double> Residual(const std::vector<double>& value) const override
    {
        std::vector<double> residual;
        residual.reserve((controlPoints.size() - 2) * 2);

        for (std::size_t i = 1; i + 1 < controlPoints.size(); ++i)
        {
            const Vec2 prev = { value[(i - 1) * 2], value[(i - 1) * 2 + 1] };
            const Vec2 curr = { value[i * 2], value[i * 2 + 1] };
            const Vec2 next = { value[(i + 1) * 2], value[(i + 1) * 2 + 1] };

            const Vec2 delta = { curr.x - 0.5 * (prev.x + next.x), curr.y - 0.5 * (prev.y + next.y) };
            residual.push_back(delta.x);
            residual.push_back(delta.y);
        }

        return residual;
    }

    std::vector<std::vector<double>> Jacobian(const std::vector<double>& value) const override
    {
        const std::size_t count = controlPoints.size();
        const std::size_t n = count * 2;
        std::vector<std::vector<double>> jacobian(n, std::vector<double>(n, 0.0));

        for (std::size_t i = 0; i < count; ++i)
        {
            const std::size_t row = i * 2;
            const std::size_t colX = i * 2;
            const std::size_t colY = i * 2 + 1;

            if (i > 0)
            {
                jacobian[row][colX - 2] = -0.5;
                jacobian[row + 1][colY - 2] = -0.5;
            }
            if (i < count - 1)
            {
                jacobian[row][colX + 2] = -0.5;
                jacobian[row + 1][colY + 2] = -0.5;
            }
            jacobian[row][colX] = 1.0;
            jacobian[row + 1][colY] = 1.0;
        }

        (void)value;
        return jacobian;
    }

    std::vector<double> InitialGuess() const override
    {
        std::vector<double> values;
        values.reserve(controlPoints.size() * 2);
        for (const auto& point : controlPoints)
        {
            values.push_back(point.x);
            values.push_back(point.y);
        }
        return values;
    }

    std::vector<double> CurrentState() const override
    {
        return InitialGuess();
    }

    void SetState(const std::vector<double>& value) override
    {
        controlPoints.clear();
        for (std::size_t i = 0; i + 1 < value.size(); i += 2)
        {
            controlPoints.emplace_back(value[i], value[i + 1]);
        }
    }
};

class BezierCurve2D
{
public:
    std::vector<Vec2> controlPoints;

    BezierCurve2D() = default;
    explicit BezierCurve2D(std::vector<Vec2> points) : controlPoints(std::move(points)) {}

    Vec2 Evaluate(double t) const
    {
        if (controlPoints.empty())
            return {};

        std::vector<Vec2> points = controlPoints;
        while (points.size() > 1)
        {
            std::vector<Vec2> next;
            next.reserve(points.size() - 1);
            for (std::size_t i = 0; i + 1 < points.size(); ++i)
            {
                next.emplace_back(
                    points[i].x + t * (points[i + 1].x - points[i].x),
                    points[i].y + t * (points[i + 1].y - points[i].y));
            }
            points = std::move(next);
        }

        return points.front();
    }

    Vec2 Derivative(double t) const
    {
        if (controlPoints.size() < 2)
            return { 0.0, 0.0 };

        std::vector<Vec2> points = controlPoints;
        while (points.size() > 1)
        {
            std::vector<Vec2> next;
            next.reserve(points.size() - 1);
            for (std::size_t i = 0; i + 1 < points.size(); ++i)
            {
                next.emplace_back(
                    points[i].x + t * (points[i + 1].x - points[i].x),
                    points[i].y + t * (points[i + 1].y - points[i].y));
            }
            points = std::move(next);
        }

        // derivative approximation based on first-order difference of the control polygon
        if (controlPoints.size() >= 2)
        {
            const auto& p0 = controlPoints.front();
            const auto& p1 = controlPoints.back();
            return { (p1.x - p0.x), (p1.y - p0.y) };
        }

        return { 0.0, 0.0 };
    }
};

class BezierIntersectionProblem : public MathProblem
{
public:
    BezierCurve2D curveA;
    BezierCurve2D curveB;
    std::vector<double> x; // [t, u]
    double tolerance = 1e-8;
    int maxIterations = 50;

    BezierIntersectionProblem() = default;

    BezierIntersectionProblem(
        BezierCurve2D a,
        BezierCurve2D b,
        std::vector<double> initialGuess,
        double tol = 1e-8,
        int maxItr = 50)
        : curveA(std::move(a)), curveB(std::move(b)), x(std::move(initialGuess)), tolerance(tol), maxIterations(maxItr)
    {
    }

    std::string Name() const override { return "BezierIntersectionProblem"; }

    bool IsValid() const override
    {
        return !curveA.controlPoints.empty() && !curveB.controlPoints.empty() && x.size() >= 2;
    }

    void Reset() override
    {
        x.clear();
    }

    std::vector<double> Residual(const std::vector<double>& value) const override
    {
        const Vec2 pa = curveA.Evaluate(value[0]);
        const Vec2 pb = curveB.Evaluate(value[1]);
        return { pa.x - pb.x, pa.y - pb.y };
    }

    std::vector<std::vector<double>> Jacobian(const std::vector<double>& value) const override
    {
        const Vec2 da = curveA.Derivative(value[0]);
        const Vec2 db = curveB.Derivative(value[1]);
        return {
            { -da.x, db.x },
            { -da.y, db.y }
        };
    }

    std::vector<double> InitialGuess() const override
    {
        return x;
    }

    std::vector<double> CurrentState() const override
    {
        return x;
    }

    void SetState(const std::vector<double>& value) override
    {
        x = value;
    }
};

} // namespace Math
} // namespace CGAlgo

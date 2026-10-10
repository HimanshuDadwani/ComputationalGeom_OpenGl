#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "DebugVisualizer.h"
#include "MathProblem.h"

namespace CGAlgo {
namespace Math {

inline double ComputeResidualNorm(const std::vector<double>& residual)
{
    if (residual.empty())
        return 0.0;

    double maxAbs = 0.0;
    for (double value : residual)
        maxAbs = std::max(maxAbs, std::abs(value));
    return maxAbs;
}

inline std::vector<std::vector<double>> BuildIdentityMatrix(std::size_t size)
{
    std::vector<std::vector<double>> identity(size, std::vector<double>(size, 0.0));
    for (std::size_t i = 0; i < size; ++i)
        identity[i][i] = 1.0;
    return identity;
}

inline std::vector<double> SolveLinearSystem(const std::vector<std::vector<double>>& matrix, const std::vector<double>& rhs)
{
    if (matrix.empty())
        return {};

    const std::size_t n = matrix.size();
    if (matrix[0].size() != n || rhs.size() != n)
        throw std::invalid_argument("SolveLinearSystem requires a square Jacobian and matching residual vector.");

    std::vector<std::vector<double>> augmented = matrix;
    std::vector<double> solution = rhs;

    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t pivotRow = i;
        double pivotValue = std::abs(augmented[i][i]);

        for (std::size_t row = i + 1; row < n; ++row)
        {
            const double candidate = std::abs(augmented[row][i]);
            if (candidate > pivotValue)
            {
                pivotValue = candidate;
                pivotRow = row;
            }
        }

        if (pivotValue <= std::numeric_limits<double>::epsilon())
            throw std::runtime_error("Jacobian is singular or ill-conditioned.");

        if (pivotRow != i)
        {
            std::swap(augmented[i], augmented[pivotRow]);
            std::swap(solution[i], solution[pivotRow]);
        }

        const double pivot = augmented[i][i];
        for (std::size_t col = i; col < n; ++col)
            augmented[i][col] /= pivot;
        solution[i] /= pivot;

        for (std::size_t row = 0; row < n; ++row)
        {
            if (row == i)
                continue;

            const double factor = augmented[row][i];
            if (std::abs(factor) <= std::numeric_limits<double>::epsilon())
                continue;

            for (std::size_t col = i; col < n; ++col)
                augmented[row][col] -= factor * augmented[i][col];
            solution[row] -= factor * solution[i];
        }
    }

    return solution;
}

class MathSolverStrategy
{
public:
    virtual ~MathSolverStrategy() = default;
    virtual SolverResult Solve(MathProblem& problem) = 0;
    virtual std::string Name() const = 0;
};

class NewtonSolverStrategy : public MathSolverStrategy
{
public:
    std::string Name() const override { return "Newton"; }

    SolverResult Solve(MathProblem& problem) override
    {
        SolverResult result;
        result.x = problem.CurrentState();

        const std::size_t variableCount = result.x.size();
        if (variableCount == 0)
        {
            result.message = "No variables were supplied.";
            return result;
        }

        for (int iteration = 0; iteration < 50; ++iteration)
        {
            const std::vector<double> residual = problem.Residual(result.x);
            const double residualNorm = ComputeResidualNorm(residual);
            result.residualNorm = residualNorm;
            result.iterations = iteration + 1;

            if (residualNorm < problem.IsValid() ? 1e-8 : 1e-8)
            {
                result.converged = true;
                result.message = "Newton method converged.";
                return result;
            }

            const auto jacobian = problem.Jacobian(result.x);
            if (jacobian.empty() || jacobian.size() != residual.size())
            {
                result.message = "Jacobian is missing or incompatible with the residual vector.";
                return result;
            }

            std::vector<double> delta;
            try
            {
                delta = SolveLinearSystem(jacobian, std::vector<double>(residual.size(), 0.0));
                for (std::size_t i = 0; i < residual.size(); ++i)
                    delta[i] = -residual[i];
                delta = SolveLinearSystem(jacobian, delta);
            }
            catch (const std::exception&)
            {
                result.message = "Newton step was singular or ill-conditioned.";
                return result;
            }

            if (delta.size() != result.x.size())
            {
                result.message = "Newton step size does not match variable count.";
                return result;
            }

            for (std::size_t i = 0; i < result.x.size(); ++i)
                result.x[i] += delta[i];

            problem.SetState(result.x);
        }

        result.message = "Newton method reached iteration limit.";
        return result;
    }
};

class GradientDescentSolverStrategy : public MathSolverStrategy
{
public:
    std::string Name() const override { return "GradientDescent"; }

    SolverResult Solve(MathProblem& problem) override
    {
        SolverResult result;
        result.x = problem.CurrentState();
        double alpha = 0.05;

        for (int iteration = 0; iteration < 200; ++iteration)
        {
            const std::vector<double> residual = problem.Residual(result.x);
            const double residualNorm = ComputeResidualNorm(residual);
            result.residualNorm = residualNorm;
            result.iterations = iteration + 1;

            if (residualNorm < 1e-8)
            {
                result.converged = true;
                result.message = "Gradient descent converged.";
                return result;
            }

            const auto jacobian = problem.Jacobian(result.x);
            std::vector<double> gradient(result.x.size(), 0.0);

            if (!jacobian.empty())
            {
                for (std::size_t row = 0; row < jacobian.size(); ++row)
                {
                    for (std::size_t col = 0; col < jacobian[row].size(); ++col)
                    {
                        gradient[col] += jacobian[row][col] * residual[row];
                    }
                }
            }
            else
            {
                // finite-difference gradient approximation for scalar-like problems
                const double eps = 1e-6;
                for (std::size_t i = 0; i < result.x.size(); ++i)
                {
                    std::vector<double> plus = result.x;
                    plus[i] += eps;
                    std::vector<double> minus = result.x;
                    minus[i] -= eps;

                    const double fPlus = ComputeResidualNorm(problem.Residual(plus));
                    const double fMinus = ComputeResidualNorm(problem.Residual(minus));
                    gradient[i] = (fPlus - fMinus) / (2.0 * eps);
                }
            }

            for (std::size_t i = 0; i < result.x.size(); ++i)
                result.x[i] -= alpha * gradient[i];

            problem.SetState(result.x);
        }

        result.message = "Gradient descent reached iteration limit.";
        return result;
    }
};

class GaussNewtonSolverStrategy : public MathSolverStrategy
{
public:
    std::string Name() const override { return "GaussNewton"; }

    SolverResult Solve(MathProblem& problem) override
    {
        SolverResult result;
        result.x = problem.CurrentState();

        for (int iteration = 0; iteration < 100; ++iteration)
        {
            const std::vector<double> residual = problem.Residual(result.x);
            const double residualNorm = ComputeResidualNorm(residual);
            result.residualNorm = residualNorm;
            result.iterations = iteration + 1;

            if (residualNorm < 1e-8)
            {
                result.converged = true;
                result.message = "Gauss-Newton converged.";
                return result;
            }

            const auto jacobian = problem.Jacobian(result.x);
            if (jacobian.empty())
            {
                result.message = "Gauss-Newton requires a Jacobian.";
                return result;
            }

            const std::size_t n = result.x.size();
            std::vector<std::vector<double>> jtj(n, std::vector<double>(n, 0.0));
            std::vector<double> jtr(n, 0.0);

            for (std::size_t i = 0; i < residual.size(); ++i)
            {
                for (std::size_t row = 0; row < n; ++row)
                {
                    jtr[row] += jacobian[i][row] * residual[i];
                    for (std::size_t col = 0; col < n; ++col)
                    {
                        jtj[row][col] += jacobian[i][row] * jacobian[i][col];
                    }
                }
            }

            std::vector<double> delta;
            try
            {
                delta = SolveLinearSystem(jtj, std::vector<double>(n, 0.0));
                for (std::size_t i = 0; i < n; ++i)
                    delta[i] = -jtr[i];
                delta = SolveLinearSystem(jtj, delta);
            }
            catch (const std::exception&)
            {
                result.message = "Normal equations are singular.";
                return result;
            }

            for (std::size_t i = 0; i < n; ++i)
                result.x[i] += delta[i];

            problem.SetState(result.x);
        }

        result.message = "Gauss-Newton reached iteration limit.";
        return result;
    }
};

} // namespace Math
} // namespace CGAlgo

#pragma once
#include <cmath>
#include <limits>
#include <iostream>
#include <functional>

using Callback = std::function<void(double)>;
using IterationFunction = std::function<double(double)>;

struct NewtonResult
{
    double x;           // Solution
    bool converged;     // Did we converge?
    int iterations;     // Iterations used
};

template<typename F, typename DF>
NewtonResult newton(
    F&& f,
    DF&& df,
    double initialGuess,
    Callback DebugCallBack = NULL,
    double tolerance = 1e-2,
    int maxIterations = 5)
{
    double x = initialGuess;

    for (int i = 0; i < maxIterations; ++i)
    {
        //std::cout << "iteration for x=" << x << std::endl;
        if (DebugCallBack)
            DebugCallBack(x);
        double fx = f(x);

        if (std::abs(fx) < tolerance)
        {
            return { x, true, i + 1 };
        }

        double dfx = df(x);

        // Avoid division by zero
        if (std::abs(dfx) < std::numeric_limits<double>::epsilon())
        {
            return { x, false, i + 1 };
        }

        x -= fx / dfx;
    }

    return { x, false, maxIterations };
}

class NewtonSolver
{
public:
    // set up the solver, can be done with the constructor
    void Setup(IterationFunction F, IterationFunction Df, double initialGuess,
        Callback dbg = NULL, double tol = 1e-2, int maxItr = 5)
    {
        m_f = (std::move(F)); 
        m_df = (std::move(Df)); 
        m_initialGuess = (initialGuess);
        m_DebugCallBack = (dbg); 
        m_tolerance = (tol); 
        m_maxIterations = (maxItr);
        m_CurrentSolutionToTest = initialGuess;
    }
    // run 1 iteration and return the intermediate result
    NewtonResult RunIteration()
    {
        double x = m_CurrentSolutionToTest;
        m_CurrIteration++;

        if (m_CurrIteration > m_maxIterations)
        {
            //std::cout << "NewtonSolver:: Max Iterations done" << x << std::endl;
            return { x, false, m_CurrIteration };
        }

        //if (m_DebugCallBack)
            //m_DebugCallBack(x);
        double fx = m_f(x);

        if (std::abs(fx) < m_tolerance)
        {
            return { x, true, m_CurrIteration };
        }

        double dfx = m_df(x);

        //std::cout << "iteration for x = " << x << " fx: " << fx << " dfx: " << dfx << std::endl;
        // Avoid division by zero
        if (std::abs(dfx) < std::numeric_limits<double>::epsilon())
        {
            return { x, false, m_CurrIteration };
            cout << "NewtonSolver:: Derivative is too small, stopping iteration." << endl;
        }

        m_CurrentSolutionToTest -= fx / dfx;

        return { x, false, m_CurrIteration };
    }

    // run all iterations and return the final reault
    NewtonResult Run()
    {
        double x = m_initialGuess;

        for (int i = 0; i < m_maxIterations; ++i)
        {
            //std::cout << "iteration for x=" << x << std::endl;
            if (m_DebugCallBack)
                m_DebugCallBack(x);
            double fx = m_f(x);

            if (std::abs(fx) < m_tolerance)
            {
                return { x, true, i + 1 };
            }

            double dfx = m_df(x);

            // Avoid division by zero
            if (std::abs(dfx) < std::numeric_limits<double>::epsilon())
            {
                return { x, false, i + 1 };
            }

            x -= fx / dfx;
        }

        return { x, false, m_maxIterations };

    }
    void Reset()
    {
        m_CurrIteration= 0;
        m_CurrentSolutionToTest = m_initialGuess;
    }
private:
    std::function<double(double)> m_f;
    std::function<double(double)> m_df;
    double m_initialGuess;
    Callback m_DebugCallBack = NULL;
    double m_tolerance = 1e-2;
    int m_maxIterations = 5;

    int m_CurrIteration = 0;
    double m_CurrentSolutionToTest = 0;
};
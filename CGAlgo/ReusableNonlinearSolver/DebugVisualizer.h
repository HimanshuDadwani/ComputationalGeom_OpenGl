#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "MathProblem.h"

namespace CGAlgo {
namespace Debug {

struct DebugTrace
{
    std::string label;
    std::vector<double> x;
    std::vector<double> residual;
};

struct SpatialDebugPoint
{
    std::string label;
    Math::Vec2 point;
};

class MathDebugVisualizer
{
public:
    void RecordIteration(const std::string& label, const std::vector<double>& x, const std::vector<double>& residual)
    {
        m_trace.push_back({ label, x, residual });
        std::cout << "[Debug] " << label << " state=";
        for (double value : x)
            std::cout << value << " ";
        std::cout << " residual=";
        for (double value : residual)
            std::cout << value << " ";
        std::cout << std::endl;
    }

    void RecordPoint(const std::string& label, const Math::Vec2& point)
    {
        m_points.push_back({ label, point });
        std::cout << "[Debug] point " << label << " = (" << point.x << ", " << point.y << ")" << std::endl;
    }

    const std::vector<DebugTrace>& Trace() const { return m_trace; }
    const std::vector<SpatialDebugPoint>& Points() const { return m_points; }

private:
    std::vector<DebugTrace> m_trace;
    std::vector<SpatialDebugPoint> m_points;
};

} // namespace Debug
} // namespace CGAlgo

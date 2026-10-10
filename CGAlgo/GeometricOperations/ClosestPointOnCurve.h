#pragma once
#include "BasicGeomObjects.h"
#include"GraphicsUtils.h"
#include "vector"
#include "Algorithm.h"
//#include "NewtonSolver.h"
#include "..\CGAlgo\Solvers\NewtonSolver.h"

struct ClosestPointResult
{
    double t;
    Point pt;
    double distance;
};

class ClosestPointOnCurve// : public Algorithm
{
public:
    void Start();
    ClosestPointResult NextStep();
    ClosestPointResult Run();

    void Setup(Curve* pCurve, Point pt, double initialGuess = 0.0);
    double FindInitialGuess(Curve* pCurve, Point pt);
    void DrawDebugPointAt(double param);
private:
    // objects for rendering
    Curve* _pCurve;
    Point _InputPt;
    int m_iStep;
    std::vector<Point> _DebugPts;
    NewtonSolver m_Solver;
    VertexArray LinesVAO, PointVAO, CurveVAO;
    VertexBuffer LinesVBO, PointVBO, CurveVBO;
    IndexBuffer LinesEBO, PointEBO, CurveEBO;
};
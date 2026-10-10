#include "BezierCurve.h"
#include "MathUtilities.h"
#include "math.h"
#include"GraphicsUtils.h"
//#include "matplot/matplot.h"

double BezierCurve::BernsteinPoly::Term(short i, double t)
{
    double nFAC__BY__iFAC__TIMES__nMINUSiFAC = factorial(_numTerms) / (factorial(i)* factorial(_numTerms -i));
    return nFAC__BY__iFAC__TIMES__nMINUSiFAC * pow(t, i) * pow(1 - t, _numTerms - i);
}


Point BezierCurve::EvaluateAt(double t)
{
    int n = NumControlPoints - 1;
    Vector EvalPointVector;
    for (int i = 0; i <= n; ++i)
    {
        Vector CurrControlPointVector(ControlPoints[i]);
        EvalPointVector = EvalPointVector + (CurrControlPointVector * poly.Term(i, t));

    }
    return Point(EvalPointVector._x, EvalPointVector._y, EvalPointVector._z);
}

Vector BezierCurve::FirstDerivative(double t)
{
    if(NumControlPoints < 2) return Vector(0, 0, 0);

    int n = NumControlPoints - 1;
    int nMinus1 = n - 1;
    BernsteinPoly polyOrder_nMinus1(nMinus1);
    Vector EvalPointVector;
    for (int i = 0; i <= nMinus1 ; ++i)
    {
        Vector CurrControlPointsVector = ControlPoints[i+1] - ControlPoints[i];
        EvalPointVector = EvalPointVector + (CurrControlPointsVector * polyOrder_nMinus1.Term(i, t));

    }
    return EvalPointVector * n;
}

Vector BezierCurve::SecondDerivative(double t)
{
    if (NumControlPoints < 3) return Vector(0, 0, 0);

    int n = NumControlPoints - 1;
    int nMinus1 = n - 1;
    int nMinus2 = n - 2;
    BernsteinPoly polyOrder_nMinus2(nMinus2);
    Vector EvalPointVector;
    for (int i = 0; i <= nMinus2 ; ++i)
    {
        Vector CurrControlPointsVector = Vector(ControlPoints[i+2]) - Vector(ControlPoints[i+1]) * 2 + Vector(ControlPoints[i]);
        EvalPointVector = EvalPointVector + (CurrControlPointsVector * polyOrder_nMinus2.Term(i, t));

    }
    return EvalPointVector * n * nMinus1;
}


void BezierCurve::Draw(bool bRedraw)
{
    if (bRedraw)
    {

        DrawPoints(ControlPoints.data(), ControlPoints.size(), PointVAO, PointVBO, PointEBO, bRedraw);

        vector<Line> listOfLines;
        for (int j = 0; j < NumControlPoints - 1; ++j)
        {
            listOfLines.push_back(Line(Point(ControlPoints[j].x, ControlPoints[j].y), Point(ControlPoints[j + 1].x, ControlPoints[j + 1].y)));
        }
        DrawLinesWithVertexArray(listOfLines.data(), listOfLines.size(), LinesVAO, LinesVBO, LinesEBO, bRedraw);
        //for (int j = 0; j < NumControlPoints - 1; ++j)
        //{
        //    cout << "Bezier Curve Line: " << j << " " << *listOfLines[j] << endl;
        //    delete listOfLines[j];
        //}
        listOfLines.clear();


        float step = 0.01;
        Point start, end;
        for (double t = 0; t < 1; t = t + step)
        {
            start = EvaluateAt(t);
            end = EvaluateAt(t + step);
            listOfLines.push_back(Line(start, end));

            if (((int)(t * (1/step)) % 10) == 0)    // at 10 equal locations
            {
                end = start + (FirstDerivative(t) * 0.1);

                listOfLines.push_back(Line(start, end));

                end = start + (SecondDerivative(t) * 0.1);

                listOfLines.push_back(Line(start, end));
            }
        }

        DrawLinesWithVertexArray(listOfLines.data(), listOfLines.size(), CurveVAO, CurveVBO, CurveEBO, bRedraw);
        listOfLines.clear();
        return;
    }
    DrawPoints(NULL, 0, PointVAO, PointVBO, PointEBO, bRedraw);
    DrawLinesWithVertexArray(NULL, 0, LinesVAO, LinesVBO, LinesEBO, bRedraw);
    DrawLinesWithVertexArray(NULL, 0, CurveVAO, CurveVBO, CurveEBO, bRedraw);
    return;
}

void BezierCurve::Plot()
{
    float step = 0.01;
    Point pt;
    Vector vec;
    vector<Line> listOfLines;
    vector<double> x, x_dot, x_dot_dot;
    for (double t = 0; t < 1; t = t + step)
    {
        pt = EvaluateAt(t);
        x.push_back(pt.x);

        vec = FirstDerivative(t);
        x_dot.push_back(vec._x);

        vec = SecondDerivative(t);
        x_dot_dot.push_back(vec._x);
    }

    //std::vector<double> t = matplot::linspace(0, 1);

    //matplot::plot(t, x, "-o");
    //matplot::plot(t, x_dot, "--xr");
    //matplot::plot(t, x_dot_dot, "-:gs");
    //matplot::show();
}


//int TestBZ()
//{
//    cout << "" << endl;
//    static bool once = true;
//    static BezierCurve c;
//    if (once)
//    {
//        vector<Point> InputPoints;
//        InputPoints.push_back(Point{ 1.0, 0.2 });
//        InputPoints.push_back(Point{ 0.5, 0.2 });
//        InputPoints.push_back(Point{ 0.3, 0.8 });
//        //InputPoints.push_back(Point{ 0.1, 0.4 });
//        //InputPoints.push_back(Point{ 0.8, 0.1 });
//        //InputPoints.push_back(Point{ 0.1, 0.1 });
//        //InputPoints.push_back(Point{ 0.9, 0.9 });
//        //InputPoints.push_back(Point{ 0.5, 0.3 });
//        //InputPoints.push_back(Point{ 0.2, 0.7 });
//        //InputPoints.push_back(Point{ 0.9, 0.1 });
//        //InputPoints.push_back(Point{ 0.7, 0.4 });
//        c.SetControlPoints(InputPoints);
//        once = false;
//    }
//    c.Draw();
//    return 1;
//}
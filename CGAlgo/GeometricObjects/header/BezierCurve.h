#include "BasicGeomObjects.h"
#include <glad/glad.h>
#include "vector"
#include"GraphicsUtils.h"


class BezierCurve : public Curve
{
	class BernsteinPoly
	{
	private:
		short _numTerms; // nummber of terms
	public:
		BernsteinPoly(short numTerms=0) :_numTerms(numTerms) {}
		// returns the value of the term i for a bernstein polynomial of given n terms
		double Term(short i, double t);
		short GetNumOfTerms() { return _numTerms; }
		void SetNumOfTerms(short numTerms) { _numTerms = numTerms; }

	};
public:
	// Overload the << operator
	friend std::ostream& operator<<(std::ostream& os, const BezierCurve& l)
	{
		os << "Bezier Curve: " << "Control Point: " << l.NumControlPoints << " : " << endl;
		for (int iPt = 1; iPt <= l.NumControlPoints; ++iPt)
		{
			os << "Control Point[" << iPt << "]: (" << l.ControlPoints[iPt].x << ", " << l.ControlPoints[iPt].y << ", " << l.ControlPoints[iPt].z << ")";

		}
		return os;
	}

	// returns the curve point at paramater t, 0<= t <=1
	Point EvaluateAt(double t);
	Vector FirstDerivative(double t);
	Vector SecondDerivative(double t);
	
	void SetControlPoints(vector<Point>& Array) 
	{ 
		ControlPoints = Array; 
		NumControlPoints = ControlPoints.size(); 
		poly.SetNumOfTerms(NumControlPoints - 1);
	}
	void AddControlPoint(Point &pt)
	{
		ControlPoints.push_back(pt);
		NumControlPoints++;
		poly.SetNumOfTerms(NumControlPoints - 1);
	}
	void Draw(bool bRedraw = false);
	void Plot();

	static void TestBerizer()
	{
		BezierCurve curve;
		vector<Point> Array{ Point(-1,0,0), Point(0,1,0), Point(1,0,0), Point(2,-1,0), Point(3,0,0) };

		curve.SetControlPoints(Array);

		double t = 0.08, h = 0.001;
		for (int i = 0; i < 5; i++, t += 0.08)
		{
			Vector firstDerivatve = curve.FirstDerivative(t);
			Vector approxFirstDeriv = (Vector(curve.EvaluateAt(t + h)) - Vector(curve.EvaluateAt(t - h))) * (1 / (2 * h));
			double difference = (firstDerivatve - approxFirstDeriv).Magnitude();
			cout << "TestBerizer: first derivative difference at t = " << t << " is " << difference << endl;
			ASSERT(abs(difference) < 0.01);

			Vector secondDerivatve = curve.SecondDerivative(t);
			Vector approxsecondDeriv = (Vector(curve.EvaluateAt(t + h)) - Vector(curve.EvaluateAt(t)) * 2
				+ Vector(curve.EvaluateAt(t - h))) * (1 / pow(h, 2));
			difference = (secondDerivatve - approxsecondDeriv).Magnitude();
			cout << "TestBerizer: second derivative difference at t = " << t << " is " << difference << endl;
			ASSERT(abs(difference) < 0.01);

		}

	}

private:
	short NumControlPoints;
	vector<Point> ControlPoints;

	// Bezier Curve is defnied by interpolation function and the control points
	// the curve doe not pass through the control points
	// it uses bernstien polynomials defined by
	//	Bi,n(t) = n!/i!(n-i)! * t^i * (1-t)^n-i
	// so i need a the terms of bernstein polynomials
	// does not 
	BernsteinPoly poly;


private:
	// objects for rendering
	VertexArray LinesVAO, PointVAO, CurveVAO;
	VertexBuffer LinesVBO, PointVBO, CurveVBO;
	IndexBuffer LinesEBO, PointEBO, CurveEBO;
};

// extern int TestBZ();

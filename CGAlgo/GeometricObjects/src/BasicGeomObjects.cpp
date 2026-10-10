#include "BasicGeomObjects.h"


void Line::GetIntersection(const Line* l, Point& intPt, bool lineMode) const
{
	const Line& L1 = *this;
	const Line& L2 = *l;
	// first set up the equations for the 2 lines
	// line eqns in parametric form : x = x1 + (x2-x1)t and y = y1 + (y2-y1)t
	// line 1 - x = a1t1 + b1 and y = c1t1 + d1
	// line 2 - x = a2t2 + b2 and y = c2t2 + d2
	// 
	double a1 = L1.End.x - L1.St.x;
	double b1 = L1.St.x;
	double c1 = L1.End.y - L1.St.y;
	double d1 = L1.St.y;

	double a2 = L2.End.x - L2.St.x;
	double b2 = L2.St.x;
	double c2 = L2.End.y - L2.St.y;
	double d2 = L2.St.y;

	// 
  // at intersection, x and y of both lines will be same
  // => a1t1 + b1 = a2t2 + b2 => a1t1 - a2t2 + b1 - b2 = 0
  // and c1t1 + d1 = c2t2 + d2 => c1t1 - c2t2 + d1 - d2 = 0
  // 
  // these are the linear equations to be solved
  // A1X+B1Y+C1=0 and A2X+B2Y+C2=0
  // 

	double A1 = a1, B1 = -a2, C1 = b1 - b2;
	double A2 = c1, B2 = -c2, C2 = d1 - d2;

	// solution of the two linear equation, by Cramer's rule, and also when solved simultaneously by hand
	// X = (C2B1-C1B2)/(A1B2-A2B1)	|	Y = (A2C1-A1C2)/(A1B2-A2B1)
	double detCoeffMatrix = (A1 * B2) - (A2 * B1);
	if (0 != detCoeffMatrix)
	{
		double t1 = ((C2 * B1) - (C1 * B2)) / detCoeffMatrix;
		double t2 = ((C1 * A2) - (C2 * A1)) / detCoeffMatrix;

		if ((0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1) || lineMode)
		{
			// so intersection point is 
			intPt.x = (a1 * t1) + b1;
			intPt.y = (c1 * t1 + d1);
			cout << "t1: " << t1 << " t2: " << t2 << endl;
		}
		else
			cout << "No Intersection between: " << L1 << " and " << L2 << endl;
	}
	else
		cout << "Determinant of Coeff Matrix is 0" << endl;
}

Vector::Vector(const Point& pt) : _x(pt.x), _y(pt.y), _z(pt.z) {}


#pragma once

#include "iostream"
using namespace std;
class Point;

class Vector
{
public:
	double _x, _y, _z;
	void Normalize()
	{
		double magnitude = sqrt(_x * _x + _y * _y + _z * _z);
		_x /= magnitude;
		_y /= magnitude;
		_z /= magnitude;
	}
	double Magnitude()
	{
		return sqrt(_x * _x + _y * _y + _z * _z);
	}
	Vector(double x =0 , double y =0, double z=0) :_x(x), _y(y), _z(z) {}
	Vector(const Point& pt);// : _x(pt.x), _y(pt.y), _z(pt.z) {} // forward declaring Point class is not enough, since variables are unknown, hence moving the definition of this constructor below Point clasdd
	Vector operator * (double scale)
	{
		return Vector{ _x * scale, _y * scale, _z * scale };
	}

	double operator * (Vector other)
	{
		return _x * other._x + _y * other._y + _z * other._z;
	}

	Vector operator + (const Vector& other)
	{
		return Vector{ other._x + _x, other._y + _y, other._z + _z };
	}

	Vector operator - (const Vector& other)
	{
		return Vector{ _x - other._x, _y - other._y , _z - other._z };
	}

	// Overload the << operator
	friend std::ostream& operator<<(std::ostream& os, const Vector& p) {

		os << "Vector(" << p._x << ", " << p._y << ", " << p._z << ")";

		return os;
	}
};

class Point {

public:
	double x, y, z;

	Point(double _x, double _y, double _z=0) :x(_x), y(_y), z(0) {};
	Point(const Point& pt) :x(pt.x), y(pt.y), z(pt.z) {};
	Point() :x(0), y(0), z(0) {};

	friend bool operator== (const Point& first, const Point& other) { return first.x == other.x && first.y == other.y && first.z == other.z; };

	Vector operator - (const Point& other)
	{
		return Vector{ x - other.x, y - other.y, z - other.z };
	}

	Point operator + (const Vector& vec)
	{
		return Point{ x + vec._x, y + vec._y, z + vec._z };
	}

	// Overload the << operator
	friend std::ostream& operator<<(std::ostream& os, const Point& p) {

		os << "Point(" << p.x << ", " << p.y << ", " << p.z << ")";

		return os;
	}
};

class Line {
public:
	Point St, End;
	Line(Point start, Point end) : St(start), End(end) {}

	// intersection
	void GetIntersection(const Line* l, Point& intPt, bool lineMode = false) const;
	// Overload the << operator
	friend std::ostream& operator<<(std::ostream& os, const Line& l)
	{
		os << "Line: " << "Start " << l.St << " End " << l.End;
		return os;
	}
};

typedef  Line;

class Curve
{
public:
	virtual Point EvaluateAt(double t) = 0;
	virtual Vector FirstDerivative(double t) = 0;
	virtual Vector SecondDerivative(double t) = 0;
};
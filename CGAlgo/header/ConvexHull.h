#pragma once
// point object
// line object
// check turn method for 2 lines using dot product, using which convext hull points are found

// sorting the points - merge sort

#include "vector"
#include "iostream"
#include <glad/glad.h>

using namespace std;

#include "BasicGeomObjects.h"
int MergeSort(vector<Point>& array, int left, int right);

extern class ConvexHull
{
private:
	double CheckTurnMethod(Line Line1, Line Line2);	// return +ve for left turn, -ve for right turn, 0 for co-linear
public:
	ConvexHull() :LineVBO(NULL), PointVBO(NULL) {};
	~ConvexHull();
	vector<Point> ComputeConvexHull();
	void SetInputPoints(vector<Point>& Array) { InputPoints = Array; }
	void Draw();

private:
	GLuint* LineVBO;
	GLuint* PointVBO;
	vector<Point> InputPoints;
	vector<Point> CHPoints;
};


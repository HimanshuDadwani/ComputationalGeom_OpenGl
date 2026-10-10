// point object
// line object
// check turn method for 2 lines using dot product, using which convext hull points are found

// sorting the points - merge sort

#include "BasicGeomObjects.h"
#include "ConvexHull.h"
#include "GraphicsUtils.h"

using namespace std;

// return +ve for left turn, -ve for right turn, 0 for co-linear
double ConvexHull::CheckTurnMethod(Line Line1, Line Line2)
{
	Vector V1 = Line1.End - Line1.St;
	Vector V2 = Line2.End - Line2.St;
	V1.Normalize();
	V2.Normalize();

	// taking cross product, assuming z component is always 0
	return (V1._x * V2._y) - (V1._y * V2._x);
}

int MergeSort(vector<Point>& array, int left, int right)
{
	// if sub array is or size = 1, it cannot be divided
	// hence it is sorted, return
	if (left >= right)
	{
		return 0;
	}

	// divide the array into sub arrays
	int mid = left + (right - left) / 2;
	MergeSort(array, left, mid);
	MergeSort(array, mid + 1, right);

	// Merge the two sorted sub arrays;
	// modifying the same array, hence first store the sub arrays in temp arrays

	int iSizeArr1 = mid - left + 1, iSizeArr2 = right - mid;
	vector<Point> subArr1(array.begin() + left, array.begin() + mid + 1/*iterator should point to next obj*/)
		, subArr2(array.begin() + mid + 1, array.begin() + right + 1);

	int i = 0, j = 0, k = 0;
	while (i < iSizeArr1 && j < iSizeArr2)
	{
		if (subArr1[i].x < subArr2[j].x)
		{
			array[left + k] = subArr1[i];
			i++;
			k++;
		}
		else
		{
			array[left + k] = subArr2[j];
			j++;
			k++;
		}
	}

	for (; i < iSizeArr1; i++, k++) {
		array[left + k] = subArr1[i];
	}

	for (; j < iSizeArr2; j++, k++) {
		array[left + k] = subArr2[j];
	}

	return 0;
}

vector<Point> ConvexHull::ComputeConvexHull()
{
	//vector<Point> InputPoints = { {10,2,0}, { 5,7, 0 },{1,0,0},{4,4,0}, {9,1,0} };
	MergeSort(InputPoints, 0, InputPoints.size() - 1);
	cout << "Sorted Array: ";

	for (int i = 0; i < InputPoints.size(); ++i)
	{
		cout << InputPoints[i].x << ", ";
	}

	cout << endl;

	// points are sorted in x coord
	// iterate through the points
	CHPoints.push_back(InputPoints[0]);
	CHPoints.push_back(InputPoints[1]);

	for (int i = 2; i < InputPoints.size(); ++i)
	{
		// check if point i can be inserted in the convex hull array
		bool Checking = true;
		int CurrIndex = CHPoints.size() - 1;

		while (Checking && CurrIndex > 0)
		{
			Line L1(CHPoints[CurrIndex - 1], CHPoints[CurrIndex]);
			Line L2(CHPoints[CurrIndex], InputPoints[i]);
			double Turn = CheckTurnMethod(L1, L2);

			if (Turn <= 0) // turns right, hence this point is to be added to convex hull list
			{
				Checking = false;
			}
			else
			{
				// need to add to this point to convex hull and remove the last one in the list
				CHPoints.pop_back();
				CurrIndex--;
			}

		}
		CHPoints.push_back(InputPoints[i]);
	}

	// iterate through the points in reverse order, such the we can get the below half of convex hull
	vector<Point> ConvexHullBelow;
	int NumPoints = InputPoints.size();
	ConvexHullBelow.push_back(InputPoints[NumPoints - 1]);
	ConvexHullBelow.push_back(InputPoints[NumPoints - 2]);

	for (int i = NumPoints - 3; i >= 0; --i)
	{
		// check if point i can be inserted in the convex hull array
		bool Checking = true;
		int CurrIndex = ConvexHullBelow.size() - 1;

		while (Checking && CurrIndex > 0)
		{
			Line L1(ConvexHullBelow[CurrIndex - 1], ConvexHullBelow[CurrIndex]);
			Line L2(ConvexHullBelow[CurrIndex], InputPoints[i]);
			double Turn = CheckTurnMethod(L1, L2);

			if (Turn <= 0) // turns right, hence this point is to be added to convex hull list
			{
				Checking = false;
			}
			else
			{
				// need to add to this point to convex hull and remove the last one in the list
				ConvexHullBelow.pop_back();
				CurrIndex--;
			}

		}
		ConvexHullBelow.push_back(InputPoints[i]);
	}

	for (int i = 1; i < ConvexHullBelow.size(); ++i)
	{
		CHPoints.push_back(ConvexHullBelow[i]);
	}

	LineVBO = new GLuint[CHPoints.size() - 1];
	//PointVBO = new GLuint[InputPoints.size()];
	PointVBO = new GLuint[1];
	GLCall(glGenBuffers(CHPoints.size() - 1, LineVBO));
	//glGenBuffers(InputPoints.size(), PointVBO);
	GLCall(glGenBuffers(1, PointVBO));

	return CHPoints;
}

void ConvexHull::Draw()
{
	DrawPoints(InputPoints.data(), InputPoints.size(), *PointVBO);
	
	for (int j = 0; j < CHPoints.size() - 1; ++j)
	{
		Line line(Point(CHPoints[j].x, CHPoints[j].y), Point(CHPoints[j + 1].x, CHPoints[j + 1].y));
		DrawLine(line, LineVBO[j]);
	}


}

ConvexHull::~ConvexHull()
{
	if (PointVBO)
	{
		GLCall(glDeleteBuffers(1, PointVBO));
	}

	if (CHPoints.size())
	{
		GLCall(glDeleteBuffers(CHPoints.size() - 1, LineVBO));
	}
}

int TestConvexHull()
{
	vector<Point> InputPoints;
	InputPoints.push_back(Point{ 1.0, 0.2 });
	InputPoints.push_back(Point{ 0.5, 0.2 });
	InputPoints.push_back(Point{ 0.3, 0.8 });
	InputPoints.push_back(Point{ 0.1, 0.4 });
	InputPoints.push_back(Point{ 0.8, 0.1 });
	InputPoints.push_back(Point{ 0.1, 0.1 });
	InputPoints.push_back(Point{ 0.9, 0.9 });
	InputPoints.push_back(Point{ 0.5, 0.3 });
	InputPoints.push_back(Point{ 0.2, 0.7 });
	InputPoints.push_back(Point{ 0.9, 0.1 });
	InputPoints.push_back(Point{ 0.7, 0.4 });
	ConvexHull c;
	c.SetInputPoints(InputPoints);
	vector<Point> ConvexHullPoints = c.ComputeConvexHull();

	c.Draw();

	return 1;
}
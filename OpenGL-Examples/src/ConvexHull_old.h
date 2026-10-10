//#pragma once
//// point object
//// line object
//// check turn method for 2 lines using dot product, using which convext hull points are found
//
//// sorting the points - merge sort
//
//#include "vector"
//#include "iostream"
////#include "math"
//
//using namespace std;
//
//class Vector
//{
//public:
//	double x, y, z;
//	void Normalize()
//	{
//		double magnitude = sqrt(x * x + y * y + z * z);
//		x /= magnitude;
//		y /= magnitude;
//		z /= magnitude;
//	}
//};
//
//class Point {
//public:
//	double x, y, z;
//	Vector operator - (const Point& other)
//	{
//		return Vector{ other.x - x, other.y - y, other.z - z };
//	}
//};
//
//class Line {
//public:
//	Line(Point pt1, Point pt2) : St(pt1), End(pt2) {}
//	Point St, End;
//};
//
//// return +ve for left turn, -ve for right turn, 0 for co-linear
//double CheckTurnMethod(Line Line1, Line Line2)
//{
//	Vector V1 = Line1.End - Line1.St;
//	Vector V2 = Line2.End - Line2.St;
//	V1.Normalize();
//	V2.Normalize();
//
//	// taking cross product, assuming z component is always 0
//	return (V1.x * V2.y) - (V1.y * V2.x);
//}
//
//int MergeSort(vector<Point>& array, int left, int right)
//{
//	// if sub array is or size = 1, it cannot be divided
//	// hence it is sorted, return
//	if (left >= right)
//	{
//		return 0;
//	}
//
//	// divide the array into sub arrays
//	int mid = left + (right - left) / 2;
//	MergeSort(array, left, mid);
//	MergeSort(array, mid + 1, right);
//
//	// Merge the two sorted sub arrays;
//	// modifying the same array, hence first store the sub arrays in temp arrays
//
//	int iSizeArr1 = mid - left + 1, iSizeArr2 = right - mid;
//	vector<Point> subArr1(array.begin() + left, array.begin() + mid + 1/*iterator should point to next obj*/)
//		, subArr2(array.begin() + mid + 1, array.begin() + right + 1);
//
//	int i = 0, j = 0, k = 0;
//	while (i < iSizeArr1 && j < iSizeArr2)
//	{
//		if (subArr1[i].x < subArr2[j].x)
//		{
//			array[left + k] = subArr1[i];
//			i++;
//			k++;
//		}
//		else
//		{
//			array[left + k] = subArr2[j];
//			j++;
//			k++;
//		}
//	}
//
//	for (; i < iSizeArr1; i++, k++) {
//		array[left + k] = subArr1[i];
//	}
//
//	for (; j < iSizeArr2; j++, k++) {
//		array[left + k] = subArr2[j];
//	}
//
//	return 0;
//}
//
// vector<Point> ComputeConvexHull(vector<Point>& Array)
//{
//	//vector<Point> Array = { {10,2,0}, { 5,7, 0 },{1,0,0},{4,4,0}, {9,1,0} };
//	MergeSort(Array, 0, Array.size() - 1);
//	cout << "Sorted Array: ";
//
//	for (int i = 0; i < Array.size(); ++i)
//	{
//		cout << Array[i].x << ", ";
//	}
//
//	cout << endl;
//
//	// points are sorted in x coord
//	// iterate through the points
//	vector<Point> ConvexHull;
//	ConvexHull.push_back(Array[0]);
//	ConvexHull.push_back(Array[1]);
//
//	for (int i = 2; i < Array.size(); ++i)
//	{
//		// check if point i can be inserted in the convex hull array
//		bool Checking = true;
//		int CurrIndex = ConvexHull.size() - 1;
//
//		while (Checking && CurrIndex > 0)
//		{
//			Line L1(ConvexHull[CurrIndex - 1], ConvexHull[CurrIndex]);
//			Line L2(ConvexHull[CurrIndex], Array[i]);
//			double Turn = CheckTurnMethod(L1, L2);
//
//			if (Turn <= 0) // turns right, hence this point is to be added to convex hull list
//			{
//				Checking = false;
//			}
//			else
//			{
//				// need to add to this point to convex hull and remove the last one in the list
//				ConvexHull.pop_back();
//				CurrIndex--;
//			}
//
//		}
//		ConvexHull.push_back(Array[i]);
//	}
//
//	// iterate through the points in reverse order, such the we can get the below half of convex hull
//	vector<Point> ConvexHullBelow;
//	int NumPoints = Array.size();
//	ConvexHullBelow.push_back(Array[NumPoints - 1]);
//	ConvexHullBelow.push_back(Array[NumPoints - 2]);
//
//	for (int i = NumPoints - 3; i >= 0; --i)
//	{
//		// check if point i can be inserted in the convex hull array
//		bool Checking = true;
//		int CurrIndex = ConvexHullBelow.size() - 1;
//
//		while (Checking && CurrIndex > 0)
//		{
//			Line L1(ConvexHullBelow[CurrIndex - 1], ConvexHullBelow[CurrIndex]);
//			Line L2(ConvexHullBelow[CurrIndex], Array[i]);
//			double Turn = CheckTurnMethod(L1, L2);
//
//			if (Turn <= 0) // turns right, hence this point is to be added to convex hull list
//			{
//				Checking = false;
//			}
//			else
//			{
//				// need to add to this point to convex hull and remove the last one in the list
//				ConvexHullBelow.pop_back();
//				CurrIndex--;
//			}
//
//		}
//		ConvexHullBelow.push_back(Array[i]);
//	}
//
//	for (int i = 1; i < ConvexHullBelow.size(); ++i)
//	{
//		ConvexHull.push_back(ConvexHullBelow[i]);
//	}
//
//	return ConvexHull;
//}
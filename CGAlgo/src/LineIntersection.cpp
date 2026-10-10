//#include "iostream"
//using namespace std;
//#include <cstdlib>
//#include <ctime>
//#include <vector>
//#include <set>
//#include <algorithm>
//#include "BasicGeomObjects.h"
//#include "GraphicsUtils.h"
//
//Point CurrentEventPoint;
//
//class LineComparator_X {
//public:
//	bool operator() (Line a, Line b) const {
//		// intersection of these lines with a horizontal line passing just below CurrentEventPoint
//		double perturbation = 0.01;
//		Line sweepL = Line(Point(0, CurrentEventPoint.y - perturbation), Point(100, CurrentEventPoint.y - perturbation));
//		Point Inta, Intb;
//		sweepL.GetIntersection(&a, Inta, true);
//		sweepL.GetIntersection(&b, Intb, true);
//		return Inta.x < Intb.x;
//	}
//};
//
//class LineComparator_Y {
//	bool operator() (Line a, Line b) {
//		// intersection of these lines with a horizontal line passing throught CurrentEventPoint
//		Line sweepL = Line(Point(0, CurrentEventPoint.y), Point(100, CurrentEventPoint.y));
//		Point Inta, Intb;
//		sweepL.GetIntersection(&a, Inta);
//		sweepL.GetIntersection(&b, Intb);
//		return Inta.y < Intb.y;
//	}
//};
// 
//enum EventType { Start, Intersection, End };
//
//class Event {
//public:
//	Point m_EventPoint;
//	// event type
//	EventType m_type;
//	vector<const Line> m_LinesAtEvent;
//	Event(Point pt, EventType t, vector<const Line> lines) : m_EventPoint(pt), m_type(t) {
//		for (const Line l : lines)
//			m_LinesAtEvent.push_back(l);
//	};
//};
//
//struct EventCompare {
//	bool operator() (Event* first, Event* other) const {
//		bool isThisEventLess = false;
//		if (first->m_EventPoint.y > other->m_EventPoint.y)
//			isThisEventLess = true;
//		else if (first->m_EventPoint.y == other->m_EventPoint.y && first->m_EventPoint.x < other->m_EventPoint.x)
//			isThisEventLess = true;
//		else if (first->m_EventPoint == other->m_EventPoint) {
//			if (other->m_type == EventType::Start) {
//				for (const Line l : other->m_LinesAtEvent)
//					first->m_LinesAtEvent.push_back(l);
//			}
//		}
//		return isThisEventLess;
//	}
//};
//
//class SweepLine {
//public:
//	SweepLine() {};
//	~SweepLine() { LinesVAO.UnBind();  EventsVAO.UnBind(); };
//	set<Line, LineComparator_X> LinesIntersSweepLines;
//	set <Event*, EventCompare> EventList;
//	// this could be a max/min heap since we only need to pop one event with max y, at a time, but we want the events to be unique, hence we need set, which is sorted, and hence can also disallow same elements
//	void ComputeIntersections();
//	void SetListOfLines(vector<Line>& Lines);
//	void BruteFroceCompute();
//	void Draw();
//	//data
//	vector<Line> m_ListofLines;
//	VertexArray LinesVAO, EventsVAO;
//	VertexBuffer LinesVBO, EventsVBO;
//	IndexBuffer LinesEBO, EventsEBO;
//};
//
//void SweepLine::BruteFroceCompute() {
//	vector<Point> IntersctionPoints;
//	for (int i = 0; i < m_ListofLines.size(); ++i) {
//		Line* L1 = &(m_ListofLines[i]);
//		for (int j = i + 1; j < m_ListofLines.size(); ++j) {
//			Point IntPt;
//			Line* L2 = &(m_ListofLines[j]);
//			L1->GetIntersection(L2, IntPt);
//			if (!(IntPt == Point(0, 0))) {
//				IntersctionPoints.push_back(IntPt);
//				cout << "IntersectionPoints Between L1: " << *L1 << " and L2: " << *L2 << " is: " << IntPt << endl;
//			}
//		}
//	}
//}
//
//void SweepLine::ComputeIntersections()//
//{
//	vector<Point> IntersectionPoints;
//	//iterate over all events
//	set<Event*>::iterator it;
//	while (!EventList.empty())
//	{
//		it = EventList.begin();
//		cout << "Event: " << (*it)->m_EventPoint << "\n";
//		CurrentEventPoint = (*it)->m_EventPoint;
//		switch ((*it)->m_type) {
//		case EventType::Start: {
//			// add the line(s) in the event, to list of lines intersecting sweep line
//			for (int i = 0; i < (*it)->m_LinesAtEvent.size(); ++i)
//				LinesIntersSweepLines.insert((*it)->m_LinesAtEvent[i]);
//			// this list should be sorted 
//			// based on the x co - ordinate(-a small value, i.e.just after the sweep line)
//				// of intersection point between the line
//				// and a horizontal line : X = (Y coord of event point)
//				//compute the insection of this line with adjacent lines in the list
//				// add intersection point event if found and it is already not there in the list(since it is a set, only unique values are allowed)
//				// 
//				// compute intersections and append events
//			if (LinesIntersSweepLines.size() > 1)
//			{
//				// iterate over all events
//				set<Line, LineComparator_X>::iterator itLine1 = LinesIntersSweepLines.begin();
//				set<Line, LineComparator_X>::iterator itLine2 = LinesIntersSweepLines.begin();
//				itLine2++;
//
//				for (; itLine2 != LinesIntersSweepLines.end(); ++itLine1, ++itLine2)
//				{
//					Point intPt(0, 0);
//					itLine1->GetIntersection((const Line*)&(*itLine2), intPt);
//					if (!(intPt == Point(0, 0)))
//					{
//						IntersectionPoints.push_back(intPt);
//						vector<const Line> lines;
//						lines.push_back((const Line)(*itLine1));
//						lines.push_back((const Line)(*itLine2));
//						EventList.insert(new Event(intPt, EventType::Intersection, lines));
//						cout << "IntersectionPoints Between L1: " << (*itLine1) << " and L2: " << (*itLine2) << " is: " << intPt << endl;
//					}
//				}
//			}
//			break;
//		}
//		case EventType::Intersection: { break; }
//		case EventType::End: { break; }
//		default:
//			cout << "Invalid Event type " << endl;
//		}
//
//		// Start Event
//		// add the line to list of lines
//		// this list should be sorted 
//		// Based on the x co - ordinate(-a small value, i.e.just after the sweep line)
//		// of intersection point between the line
//		//	and a horizontal line : X = (Y coord of event point)
//		// compute the insection of this line with adjacent lines in the list
//		//	add intersection point event if found and it is already not there in the list(since it is a set, only unique values are allowed)
//		//
//		// Intersection Event
//		// Remove the 2 lines and the insert them back in the list, with this they will be sorted(swaped)
//		//	check the intersection of these 2 lines with adjacent lines
//		//	add intersection point event if found and it is already not there in the list
//		// 
//		// End Event
//		// Remove the line in this event
//		//	check intersection between the neighbours of this removed line which now become neighbour
//		//	add intersection point if not already added
//		//	// remove the even point, and repeat with the first event in the list
//		
//		EventList.erase(it);
//	}
//	return;
//}
//
//void SweepLine::SetListOfLines(vector<Line>& Lines) {
//	m_ListofLines = Lines;
//	// create events
//	for (int i = 0; i < Lines.size(); ++i)
//	{
//		vector<const Line> lines;
//		lines.push_back(Lines[i]);
//		if (Lines[i].St.y > Lines[i].End.y || (Lines[i].St.y == Lines[i].End.y && Lines[i].St.x < Lines[i].End.x))
//		{
//			EventList.insert(new Event(Lines[i].St, EventType::Start, lines));
//			EventList.insert(new Event(Lines[i].End, EventType::End, lines));
//		}
//		else 
//		{
//			EventList.insert(new Event(Lines[i].End, EventType::Start, lines));
//			EventList.insert(new Event(Lines[i].St, EventType::End, lines));
//		}
//	}
//
//	set<Event*>::iterator it;
//	// Loop through the vector with the iterator
//	for (it = EventList.begin(); it != EventList.end(); ++it) {
//		cout << "Event: " << (*it)->m_EventPoint << "\n";
//	}
//	return;
//}
//
//void CreateLines(vector<Line>& Lines) {
//	//srand(time(0));
//	//for (int i = 1; i <= 10; ++i)
//	//{
//	//	double x1 = -0.5 + double(rand() % 100) / 100.0;
//	//	double y1 = -0.5 + double(rand() % 100) / 100.0;
//	//	double x2 = -0.5 + double(rand() % 100) / 100.0;
//	//	double y2 = -0.5 + double(rand() % 100) / 100.0;
//	//	Line* l = new Line(Point(x1, y1), Point(x2, y2));
//	//	Lines.push_back(l);
//	//}
//
//	double x1 = -0.5, y1 = 0.5;
//	double x2 = 0.5, y2 = 0;
//	Line l =  Line(Point(x1, y1), Point(x2, y2));
//	Lines.push_back(l);
//
//	x1 = 0.2, y1 = -0.5;
//	x2 = 0.2, y2 = 0.5;
//	l = Line(Point(x1, y1), Point(x2, y2));
//	Lines.push_back(l);
//
//	x1 = 0.2, y1 = -0.5;
//	x2 = 0.2, y2 = 0.5;
//	l = Line(Point(x1, y1), Point(x2, y2));
//	Lines.push_back(l);
//}
//
//void SweepLine::Draw()
//{
//	static bool bBufferGenerated = false;
//	bool bRedraw = false;
//	if (false == bBufferGenerated)
//	{
//		bRedraw = true;
//		bBufferGenerated = true;
//	}
//	DrawLinesWithVertexArray(m_ListofLines.data(), m_ListofLines.size(), LinesVAO, LinesVBO, LinesEBO, bRedraw);
//}
//
//
//int TestLineIntersection() {
//	static SweepLine SweepL;
//	static bool bSweepLineComputed = false;
//	if (false == bSweepLineComputed)
//	{
//		srand(time(0));
//		//create list of lines
//		vector<Line> Lines;
//		CreateLines(Lines);
//
//		SweepL.SetListOfLines(Lines);
//		SweepL.BruteFroceCompute();
//		SweepL.ComputeIntersections();
//
//		for (int i = 0; i < Lines.size(); ++i) { cout << Lines[i] << endl; }
//		cout << "\n sorting \n" << endl;
//
//		sort(Lines.begin(), Lines.end(), [](const Line a, const Line b) {
//			return a.St.y < b.St.y;
//			});
//
//		for (int i = 0; i < Lines.size(); ++i) { cout << Lines[i] << endl; }
//
//		bSweepLineComputed = true;
//	}
//	SweepL.Draw();
//
//	return 1;
//}
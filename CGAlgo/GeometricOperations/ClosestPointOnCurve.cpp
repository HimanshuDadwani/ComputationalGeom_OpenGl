#include "ClosestPointOnCurve.h"
#include <string>

#define BIND_FN(x) std::bind(&ClosestPointOnCurve::x, this, std::placeholders::_1)


double  ClosestPointOnCurve::FindInitialGuess(Curve* pCurve, Point pt)
{
	double initialGuess = 0.0;

	Point curvePt = pCurve->EvaluateAt(0.0);
	Vector diff = curvePt - pt;
	double MinDist = sqrt(diff * diff);

	int step = 4;
	for (int i = 0; i <= 50; ++i)
	{
		double t = (double)i / (double)step;
		Point curvePt = pCurve->EvaluateAt(t);
		Vector diff = curvePt - pt;
		double dist = sqrt(diff * diff);
		if (dist < MinDist)
		{
			MinDist = dist;
			initialGuess = t;
		}
	
	}
	//cout << "Initial Guess: " << pCurve->EvaluateAt(initialGuess) << " t: " << initialGuess << " Dist: " << MinDist << endl;
	return initialGuess;
}
void ClosestPointOnCurve::DrawDebugPointAt(double param)
{
	_DebugPts.push_back(_pCurve->EvaluateAt(param));
	int size = _DebugPts.size();
	DrawPoints(_DebugPts.data(), _DebugPts.size(), PointVAO, PointVBO, PointEBO, true);
	Vector diff = _DebugPts[size - 1] - _InputPt;
	double MinDist = sqrt(diff * diff);
	////cout << "DebugDraw: " << _DebugPts[size-1] << " t: " << param << " Dist: " << MinDist << endl;

}
void ClosestPointOnCurve::Setup(Curve * pCurve, Point pt, double initialGuess)
{
	_DebugPts.clear();
	_pCurve = pCurve;
	_InputPt = pt;
	m_iStep = 0;
	// Define the function f(t) = (C(t) - P) . C'(t)
	// C(t) is the point on the curve at parameter t
	// C'(t) is the derivative of the curve at parameter t
	
	//Instead of minimizing the distance,
	//	d(t) = |C(t)−p|
	//	minimize the squared distance
	//	D(t) = |C(t)−p|^2
	//	This avoids the square root and has the same minimizer.

	//	Differentiate with respect to t :
	//  dD/dt = 2(C(t)−p)⋅C′	(t) =? this should be 0 at minimum distance point

	//	The optimal point satisfies
	//	(C(t)−p)⋅C′(t) = 0
	// now we want to find a t that satisfies above equation f(t) = (C(t)−p)⋅C′(t) = 0
	// now we can use newton method by adding to value, based on gradient of this function, to initial guess
	// f'(t) = C′(t)⋅C′(t) + (C(t)−p)⋅C′′(t)
	//

	//std::////cout << "Input point: " << pt << std::endl;

	auto f = [pCurve, pt](double t) {
		////cout << "f(t) called for t: " << t << endl;
		Vector C_t(pCurve->EvaluateAt(t));
		////cout << "C_t(t): " << C_t << endl;
		Vector C_prime_t = pCurve->FirstDerivative(t);
		////cout << "C_prime_t(t): " << C_prime_t << endl;
		return (C_t - Vector(pt))*(C_prime_t);
	};

	auto df = [pCurve, pt](double t) {
		Vector C_t(pCurve->EvaluateAt(t));
		Vector C_prime_t = pCurve->FirstDerivative(t);
		Vector C_double_prime_t = pCurve->SecondDerivative(t);
		//cout << "C_double_prime_t(t): " << C_double_prime_t << endl;
		return (C_prime_t) * (C_prime_t) + (C_t - Vector(pt)) * (C_double_prime_t);
		};

	//(TODO) looks like derivatives are incorrect

	if(initialGuess == 0.0)
		initialGuess = FindInitialGuess(pCurve, pt);
	DebugRenderer& dbgDraw = DebugRenderer::getInstance();
	std::string sInitGuess("Initial Guess");
	Point initPt = pCurve->EvaluateAt(initialGuess);
	dbgDraw.AddObjects(sInitGuess, &initPt, 1);
	m_Solver.Setup(f, df, initialGuess, BIND_FN(DrawDebugPointAt));

	//SetupDone(true);


};


void ClosestPointOnCurve::Start() 
{
	//NewtonResult result = newton(f, df, initialGuess, BIND_FN(DrawDebugPointAt));
	//if (result.converged == false)
	//{
	//	std:://cout << "Solution not convereged, returing initial guess: " << initialGuess << std::endl;
	//	return { result.x, pCurve->EvaluateAt(initialGuess), 0.0 };
	//}
	//else
	//{
	//	std:://cout << "Solution converged :" << result.x << std::endl;
	//	return { result.x, pCurve->EvaluateAt(result.x), 0.0 };
	//}
}

ClosestPointResult ClosestPointOnCurve::NextStep()
{
	NewtonResult result = m_Solver.RunIteration();
	DebugRenderer& dbgDraw = DebugRenderer::getInstance();
	if (result.iterations == m_iStep)
	{
		//std:://cout << "No more iterations left, returning current solution: " << result.x << std::endl;
		Point SolnPoint = _pCurve->EvaluateAt(result.x);
		std::string objName = "SolPoint:" + std::to_string(m_iStep);
		dbgDraw.AddObjects(objName, &SolnPoint, 1);
		Vector diff = SolnPoint - _InputPt;
		double distance = diff * diff;
		return { result.x, SolnPoint, distance };
	
	}
	m_iStep = result.iterations;
	if (result.converged == true)
	{
		////std:://cout << "Solution converged :" << result.x << std::endl;
		return { result.x, _pCurve->EvaluateAt(result.x), 0.0 };
	}
	else
	{
		//std:://cout << "Solution not converged :" << result.x << std::endl;
		Point SolnPoint = _pCurve->EvaluateAt(result.x);
		std::string objName = "SolPoint:" + std::to_string(m_iStep);
		dbgDraw.AddObjects(objName, &SolnPoint, 1);
		Vector diff =  SolnPoint - _InputPt;
		double distance = diff * diff;
		return { result.x, SolnPoint, distance };
	}
}

ClosestPointResult ClosestPointOnCurve::Run()
{
	m_Solver.Reset();
	NewtonResult result = m_Solver.Run();
	if (result.converged == true)
	{
		//std:://cout << "Solution converged :" << result.x << std::endl;
		return { result.x, _pCurve->EvaluateAt(result.x), 0.0 };
	}
	else
	{
		//std:://cout << "Solution not converged :" << result.x << std::endl;
		Point SolnPoint = _pCurve->EvaluateAt(result.x);
		DebugRenderer& dbgDraw = DebugRenderer::getInstance();
		std::string objName = "SolPoint:" + std::to_string(m_iStep);
		dbgDraw.AddObjects(objName, &SolnPoint, 1);
		Vector diff = SolnPoint - _InputPt;
		double distance = diff * diff;
		return { result.x, SolnPoint, distance };
	}
}
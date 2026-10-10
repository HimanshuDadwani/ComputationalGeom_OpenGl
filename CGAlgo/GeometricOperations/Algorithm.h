#pragma once
#include <iostream>
using namespace std;
class Algorithm
{
public:
	virtual void Start() = 0;
	virtual void NextStep() = 0;
	virtual void Run() = 0;
	virtual void SetupDone(bool bSetupDone) { m_SetupDone = bSetupDone; };
private:
	bool m_SetupDone;
};
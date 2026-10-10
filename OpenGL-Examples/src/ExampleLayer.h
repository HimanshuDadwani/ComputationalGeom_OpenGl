#pragma once

#include <GLCore.h>
#include "GLCore/Core/Input.h"
#include <GLCoreUtils.h>
#include <vector>
#include "ConvexHull.h"
#include "LineIntersection.h"
#include "BezierCurve.h"
#include "GeometricOperations\ClosestPointOnCurve.h"
//#include "matplot/matplot.h"

class ExampleLayer : public GLCore::Layer
{
public:
	ExampleLayer();
	virtual ~ExampleLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
	void GenerateAxes();
	void GenerateConvexHull();
	void GenerateLineIntersection();
	int GenerateBZ();

private:
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;
	GLCore::Utils::CoordinateConverter m_CoordController;
	float m_UpdateTime;
	float m_ImGuiRenderTime;

	GLuint m_AxesVA, m_AxesVB, m_AxesIB;

	glm::vec4 m_AxesBaseColor = { 0.8f, 0.2f, 0.3f, 1.0f };
	glm::vec4 m_AxesAlternateColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec4 m_AxesColor = m_AxesBaseColor;
	ClosestPointOnCurve ClosestPointOnCurveOpe;
	ConvexHull m_ConvexHull;
	BezierCurve m_BZ;
	bool m_bCreatePoint;
	//SweepLine m_SweepL;
	Point m_ClosestPoint;
	VertexArray ClosestPtPointVAO;
	VertexBuffer ClosestPtPointVBO;
	IndexBuffer ClosestPtPointEBO;

};
#pragma once

#include "OrthographicCamera.h"
#include "GLCore/Core/Timestep.h"

#include "GLCore/Events/ApplicationEvent.h"
#include "GLCore/Events/MouseEvent.h"

namespace GLCore::Utils {

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio = 1, bool rotation = true);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		OrthographicCamera& GetCamera() { return m_Camera; }
		const OrthographicCamera& GetCamera() const { return m_Camera; }

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 100.0f;
		OrthographicCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 90.0f;
	};


	class CoordinateConverter
	{
	public:

		CoordinateConverter(OrthographicCamera& camera, double width, double height);

		glm::vec2 ScreenToWorld(glm::vec2 screen);

		glm::vec2 WorldToScreen(glm::vec2 world);

		glm::vec2 ScreenToNDC(glm::vec2 screen);

		glm::vec2 NDCToScreen(glm::vec2 ndc);

		void SetViewportSize(double width, double height);
		void OnEvent(Event& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:

		OrthographicCamera& m_Camera;
		double m_Width = 1280;
		double m_Height = 720;
	};

}
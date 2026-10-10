#include "glpch.h"
#include "OrthographicCameraController.h"

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"

namespace GLCore::Utils {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel), m_Rotation(rotation)
	{
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(HZ_KEY_A))
		{
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(HZ_KEY_D))
		{
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (Input::IsKeyPressed(HZ_KEY_W))
		{
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Input::IsKeyPressed(HZ_KEY_S))
		{
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		if (m_Rotation)
		{
			if (Input::IsKeyPressed(HZ_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;
			if (Input::IsKeyPressed(HZ_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GLCORE_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}


	CoordinateConverter::CoordinateConverter(OrthographicCamera& camera, double width, double height):
		m_Camera(camera), m_Width(width), m_Height(height)
	{

	}

	glm::vec2 CoordinateConverter::ScreenToWorld(glm::vec2 screen)
	{
		glm::vec2 ndc = ScreenToNDC(screen);

		glm::vec4 clip(
			ndc.x,
			ndc.y,
			0.f,
			1.f);

		glm::vec4 world =
			m_Camera.GetInverseViewProjectionMatrix()
			* clip;

		return glm::vec2(world);
	}

	glm::vec2 CoordinateConverter::WorldToScreen(glm::vec2 world)
	{
		glm::vec4 clip =
			m_Camera.GetViewProjectionMatrix()
			*
			glm::vec4(world, 0.f, 1.f);

		glm::vec2 ndc =
		{
			clip.x / clip.w,
			clip.y / clip.w
		};

		return NDCToScreen(ndc);
	}
	
	glm::vec2 CoordinateConverter::ScreenToNDC(glm::vec2 screen)
	{
		float x =
			(2.f * screen.x) / m_Width - 1.f;

		float y =
			1.f - (2.f * screen.y) / m_Height;

		return { x,y };
	}

	glm::vec2 CoordinateConverter::NDCToScreen(glm::vec2 ndc)
	{
		float x =
			(ndc.x + 1.f) * 0.5f * m_Width;

		float y =
			(1.f - ndc.y) * 0.5f * m_Height;

		return { x,y };

	}

	void CoordinateConverter::SetViewportSize(double width, double height)
	{
		m_Width = width;
		m_Height = height;
	}

	void CoordinateConverter::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(GLCORE_BIND_EVENT_FN(CoordinateConverter::OnWindowResized));
	}

	bool CoordinateConverter::OnWindowResized(WindowResizeEvent& e)
	{
		m_Width = e.GetWidth();
		m_Height = e.GetHeight();
		return false;
	}
}
#pragma once
#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include "InvertedPendulum.h"

#define M_PI       3.14159265358979323846
#define M_PI_4     0.785398163397448309616  // pi/4

// Vertex struct
typedef struct vertex {
	float x, y, z;
} vertex;

//vertex convertCoord(int x, int y) {
//	vertex v = { (float)x / 1280 * 2 - 1, (float)y / 720 * 2 - 1, 0.0 };
//	return v;
//}

// Pendulum class
class Pendulum {
public:
	vertex center;
	vertex start;
	float length;
	float angle;
	float velocity;
	float acceleration;
	float gravity;
	float damping;
	float ipTorque;

	Pendulum() {
		center = { 0.400, 0.300, 0.0 };
		start = { 0.400, 0.500, 0.0 };
		length = 0.800;
		angle = M_PI_4;
		velocity = 0;
		acceleration = 0.0;
		gravity = 9.81;
		damping = 0.0f;
		ipTorque = 1;

		//glGenVertexArrays(1, &m_LineVAO);
		glGenBuffers(1, &m_LineVBO);


		//glGenVertexArrays(1, &m_CircleVAO);
		glGenBuffers(1, &m_CircleVBO);

	}

	~Pendulum() {
		//glDeleteVertexArrays(1, &m_CircleVAO);
		glDeleteBuffers(1, &m_CircleVBO);
		//glDeleteVertexArrays(1, &m_LineVAO);
		glDeleteBuffers(1, &m_LineVBO);
	}
	void update(double time) {
		velocity += acceleration * time;
		//velocity *= damping;
		angle += (velocity * time);
		center.x = start.x + length * sin(angle);
		center.y = start.y - length * cos(angle);
		acceleration = ((-gravity / length) * sin(angle)) + ipTorque / (length * length) - damping * velocity / length;
	}

	// Draw a line
	void drawLine() {
		float vertices[] = {
		  start.x, start.y, 0,
		  center.x, center.y, 0
		};


		//glBindVertexArray(m_LineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_LINES, 0, 2);
	}

	// Draw a circle
	void drawCircle() {
		vertex c = center;
		float radius = 0.05f;
		float vertices[360 * 3];
		for (int i = 0; i < 360; i++) {
			vertex v = { 0, 0 , 0 };
			v.x = c.x + radius * cos(i * M_PI / 180);
			v.y = c.y + radius * sin(i * M_PI / 180);
			vertices[i * 3] = v.x;
			vertices[i * 3 + 1] = v.y;
			vertices[i * 3 + 2] = v.z;
		}

		//glBindVertexArray(m_CircleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_CircleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
	}
	void SetTorque(float torque)
	{
		ipTorque = torque;
	}

	void SetDampingCoeff(float coeff)
	{
		damping = coeff;
	}

private:
	unsigned int m_CircleVBO, m_CircleVAO;
	unsigned int m_LineVBO, m_LineVAO;

};


class PendulumLayer : public GLCore::Layer
{
public:
	PendulumLayer();
	virtual ~PendulumLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;

private:
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::OrthographicCameraController m_CameraController;

	GLuint m_QuadVA, m_QuadVB, m_QuadIB;

	glm::vec4 m_SquareBaseColor = { 0.8f, 0.2f, 0.3f, 1.0f };
	glm::vec4 m_SquareAlternateColor = { 0.2f, 0.3f, 0.8f, 1.0f };
	glm::vec4 m_SquareColor = m_SquareBaseColor;
	Pendulum m_p1;
	InvertedPendulum m_p2;

};
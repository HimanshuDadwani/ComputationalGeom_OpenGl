#include "PendulumLayer.h"

using namespace GLCore;
using namespace GLCore::Utils;


PendulumLayer::PendulumLayer()
	: m_CameraController(16.0f / 9.0f),
	m_p2(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, std::vector<double> { 0 , M_PI_4, 0, 0.0})
{
}

PendulumLayer::~PendulumLayer()
{

}

void PendulumLayer::OnAttach()
{
	// create a buffer, which is a series of bytes
	glGenBuffers(1, &m_QuadVB);
	// what is this buffer of?, it is a array buffer
	// now I want to put data into it, so need first need to select it
	// selecting in openGL is called binding,
	// and we bind it to a target, which shows the purpose of this buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	// put data into it, we haven't specified size of this buffer
	// you don't need to put data straight away,
	// we can just define it's size, and put data later
	
	float positions[6] = {
		-0.5f, -0.5f,
		 0.0f, 0.5f,
		 0.5f, -0.5f
	};

	// defining the size and the data in the buffer
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float) /*this is in bytes*/
		, positions, GL_STATIC_DRAW);

	// now we need to specify the layout of the data, we gave 6 floats, \
	// but how openGL will know how to generate vertices out of it
	// it doesn't know how many coordinated per vertex
	glVertexAttribPointer(0,	// index of the this attribute, we have just 1 attribute per vrtex, which is the position, we can have normal, color, etc defined per vertex,
								// and each such info will be an attribute, this being 1st attribute ->0
		2,						// this is component count, so how many vertices are there? -> 2
		GL_FLOAT,				// type of data of this attribute
		GL_FALSE,				// do you want to normalize this, i.e. convert it to a value between 0 and 1, say for color, we have 1 byte, with value from 0 to 255, 
								// we can get it converted automatically to a float value between 0 and 1
		sizeof(float) * 2,		// stride, no. of bytes between each vertex, so 2*sizeOf(float)
		0						// pointer, this is actually a no., this is no. of bytes from start, to this attribute, if we have say color defined for each vertex
								// this would be 8, where is 8 is size of 2 floats for the 2 coordinates, and then we have color value per vertex
								// so how many bytes to get to first value for this attribute
	);

	// we also need to enable this vertex attribute
	glEnableVertexAttribArray(0); // which attrib? 0th
}

void PendulumLayer::OnDetach()
{

}

void PendulumLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			return false;
		});
}



void PendulumLayer::OnUpdate(Timestep ts)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, m_QuadVB);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	m_p1.update(ts);
	m_p1.drawLine();
	m_p1.drawCircle();

	m_p2.Update(ts,0);
	m_p2.drawLine();
	m_p2.drawCircle();
}

void PendulumLayer::OnImGuiRender()
{
	ImGui::Begin("Controls");

	static float torque = 0.0f;
	ImGui::SliderFloat("Torque", &torque, -5.0f, 5.0f);
	m_p1.SetTorque(torque);

	static float damping = 0.0f;
	ImGui::SliderFloat("damping", &damping, 0.0f, 5.0f);
	m_p1.SetDampingCoeff(damping);
	
	ImGui::End();
}

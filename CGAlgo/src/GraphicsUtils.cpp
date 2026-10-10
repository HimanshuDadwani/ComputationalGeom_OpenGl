#pragma once
#include "GraphicsUtils.h"

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR)
	{
		GLenum error = glGetError();
		cout << "OpenGL Clearing Error: " << error << endl;
	}
}

bool GLCheckError()
{
	while (GLenum error = glGetError())
	{
		cout << "OpenGL Error: " << error << endl;
		return false;
	}
	return true;
}

void DrawCircle(Point& Pt, float radius, GLuint CircleVBO)
{
	const int NumTri = 12;
	float vertices[NumTri * 3];
	for (int i = 0; i < NumTri; i++) {
		Point v = { 0, 0 };
		v.x = Pt.x + radius * cos(i * (360 / NumTri) * (M_PI / 180));
		v.y = Pt.y + radius * sin(i * (360 / NumTri) * (M_PI / 180));
		vertices[i * 3] = v.x;
		vertices[i * 3 + 1] = v.y;
		vertices[i * 3 + 2] = 0;
	}

	//glBindVertexArray(m_CircleVAO);
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, CircleVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, NumTri));
}

void DrawPoint(Point& Pt, GLuint PointVBO)
{
	float vertex[3] = { Pt.x, Pt.y, 0 };

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, PointVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW));
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glPointSize(5.0f));
	GLCall(glDrawArrays(GL_POINTS, 0, 1));
}

void DrawPoints(Point* pts, int numPts, GLuint PointVBO)
{
	float* vertices = new float[3 * numPts];
	for (int i = 0; i < numPts; ++i)
	{
		vertices[i * 3] = pts[i].x;
		vertices[i * 3 + 1] = pts[i].y;
		vertices[i * 3 + 2] = 0;
	}
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, PointVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numPts, vertices, GL_STATIC_DRAW));
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glPointSize(5.0f));
	GLCall(glDrawArrays(GL_POINTS, 0, numPts));
	delete[] vertices;

}

void DrawLine(Line& L, GLuint LineVBO)
{
	//void drawLine() {
	float vertices[] = {
	  L.St.x, L.St.y, 0,
	  L.End.x, L.End.y, 0
	};

	//glBindVertexArray(m_LineVAO);
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, LineVBO));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
	GLCall(glEnableVertexAttribArray(0));
	GLCall(glDrawArrays(GL_LINES, 0, 2));
	//}
}

void DrawLines(Line** L, int numLines, GLuint LineVAO, GLuint LineVBO, GLuint LineEBO, bool RecomputeBuffer)
{
	if (RecomputeBuffer)
	{
		float* vertices = new float[3 * 2 * numLines];

		for (int i = 0; i < numLines; ++i)
		{
			vertices[i * 6] = L[i]->St.x;
			vertices[i * 6 + 1] = L[i]->St.y;
			vertices[i * 6 + 2] = 0;
			vertices[i * 6 + 3] = L[i]->End.x;
			vertices[i * 6 + 4] = L[i]->End.y;
			vertices[i * 6 + 5] = 0;
		}

		GLCall(glBindVertexArray(LineVAO));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, LineVBO));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 2 * numLines, vertices, GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
		GLCall(glEnableVertexAttribArray(0));

		uint32_t* indices = new uint32_t[numLines * 2];
		for (int i = 0,j = 0; i < numLines; ++i)
		{
			indices[i * 2] = j++;
			indices[i * 2 + 1] = j++;
		}
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LineEBO));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * 2 * numLines, indices, GL_STATIC_DRAW));
	}

	GLCall(glBindVertexArray(LineVAO));
	GLCall(glDrawElements(GL_LINES, 2 * numLines, GL_UNSIGNED_INT, nullptr));
	GLCall(glBindVertexArray(0));

}

void DrawLinesWithVertexArray(Line* L, int numLines, VertexArray& LineVAO,
	VertexBuffer& LineVBO, IndexBuffer& LineEBO, bool RecomputeBuffer)
{
	if (RecomputeBuffer && L && numLines > 0)
	{
		float* vertices = new float[3 * 2 * numLines];
		int numCoords = 3, numPointsInLine = 2;
		for (int i = 0; i < numLines; ++i)
		{
			vertices[i * 6] =     L[i].St.x;
			vertices[i * 6 + 1] = L[i].St.y;
			vertices[i * 6 + 2] = 0;
			vertices[i * 6 + 3] = L[i].End.x;
			vertices[i * 6 + 4] = L[i].End.y;
			vertices[i * 6 + 5] = 0;
		}

		LineVAO.Bind();
		LineVBO.Bind();
		LineVBO.SetData(vertices, sizeof(float) * numCoords * numPointsInLine * numLines);
		VertexBufferLayout layout;
		layout.Push<float>(3);
		LineVAO.AddBuffer(LineVBO, layout);

		uint32_t* indices = new uint32_t[numLines * 2];
		for (int i = 0, j = 0; i < numLines; ++i)
		{
			indices[i * 2] = j++;
			indices[i * 2 + 1] = j++;
		}
		LineEBO.Bind();
		LineEBO.SetData(indices, sizeof(uint32_t) * 2 * numLines);
		delete[] vertices;
	}

	LineVAO.Bind();
	LineEBO.Bind();
	GLCall(glDrawElements(GL_LINES, LineEBO.GetCount(), GL_UNSIGNED_INT, nullptr));
	LineVAO.UnBind();
	LineEBO.UnBind();

}

void DrawPoints(Point* pts, int numPts, VertexArray& PointVAO, VertexBuffer& PointVBO, IndexBuffer& PointEBO, bool RecomputeBuffer)
{
	float* vertices = NULL;
	if (RecomputeBuffer && pts && numPts > 0)
	{
		vertices = new float[3 * numPts];
		int numCoords = 3;
		for (int i = 0; i < numPts; ++i)
		{
			vertices[i * 3] = pts[i].x;
			vertices[i * 3 + 1] = pts[i].y;
			vertices[i * 3 + 2] = 0;
		}
		PointVAO.Bind();
		PointVBO.Bind();
		PointVBO.SetData(vertices, sizeof(float) * numCoords * numPts);
		VertexBufferLayout layout;
		layout.Push<float>(3);
		PointVAO.AddBuffer(PointVBO, layout);

		uint32_t* indices = new uint32_t[numPts];
		for (int i = 0; i < numPts; ++i)
		{
			indices[i] = i;
		}
		PointEBO.Bind();
		PointEBO.SetData(indices, sizeof(uint32_t) * numPts);
		delete[] vertices;
	}
	PointVAO.Bind();
	PointEBO.Bind();
	GLCall(glPointSize(5.0f));
	GLCall(glDrawElements(GL_POINTS, PointEBO.GetCount(), GL_UNSIGNED_INT, nullptr));
	PointVAO.UnBind();

}



void VertexBuffer::UnBind()
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

VertexBuffer::VertexBuffer(void* data, unsigned int size)
{
	GLCall(glGenBuffers(1, &_ObjectId));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, _ObjectId));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	
}

VertexBuffer::~VertexBuffer()
{
	if (_ObjectId)
	{
		UnBind();
		GLCall(glDeleteBuffers(1, &_ObjectId));
		_ObjectId = 0;
	}
}

void VertexBuffer::SetData(void* data, unsigned int size)
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, _ObjectId));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	
}

void VertexBuffer::Bind()
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, _ObjectId));
}

void IndexBuffer::UnBind()
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

IndexBuffer::IndexBuffer(void* data, unsigned int size)
{
	
	GLCall(glGenBuffers(1, &_ObjectId));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ObjectId));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	

}

IndexBuffer::~IndexBuffer()
{
	if (_ObjectId)
	{
		UnBind();
		GLCall(glDeleteBuffers(1, &_ObjectId));
		_ObjectId = 0;
	}
}

void IndexBuffer::SetData(void* data, unsigned int size)
{
	_Count = size / sizeof(uint32_t);
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ObjectId));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
	
}

void IndexBuffer::Bind()
{
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ObjectId));
}

VertexArray::VertexArray()
{
	GLCall(glCreateVertexArrays(1, &_ObjectId));
}

VertexArray::~VertexArray()
{
	UnBind();
	GLCall(glDeleteVertexArrays(1, &_ObjectId));
}

void VertexArray::Bind()
{
	GLCall(glBindVertexArray(_ObjectId));
}

void VertexArray::UnBind()
{
	GLCall(glBindVertexArray(0));
}

void VertexArray::AddBuffer(VertexBuffer& vb, VertexBufferLayout& layout)
{
	
	Bind();
	vb.Bind();
	unsigned int offset = 0;
	auto attributes = layout.GetAttributes();
	for (int i = 0; i < attributes.size(); ++i)
	{
		auto attribute = attributes[i];
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, attribute.count, attribute.type, attribute.normalized, 
			layout.GetStride(), (void*)offset));
		offset += attribute.count * sizeof(attribute.type); 
	}
	

}


void Renderer::RenderPoints(Point* pts, int numPts)
{
	if (m_Type == NOT_SET)
	{
		m_Type = POINTS;
		m_Update = true;
	}

	ASSERT(m_Type == POINTS);

	if ((pts == NULL && numPts != 0) || (pts != NULL && numPts == 0))
		ASSERT(true);

	if (pts == NULL && m_Update == false)
	{
		DrawPoints(nullptr, 0, m_VAO, m_VBO, m_EBO, m_Update);
		return;
	}
	else if (pts != NULL && m_Update == true)
	{
		DrawPoints(pts, numPts, m_VAO, m_VBO, m_EBO, m_Update);
		m_Update = false;
	}
	else
		ASSERT(true);
}
void Renderer::RenderLines(Line* lines, int numLines)
{
	if (m_Type == NOT_SET)
	{
		m_Type = LINES;
		m_Update = true;
	}

	ASSERT(m_Type == LINES);

	if ((lines == NULL && numLines != 0) || (lines != NULL && numLines == 0))
		ASSERT(true);

	if (lines == NULL && m_Update == false)
	{
		DrawLinesWithVertexArray(nullptr, 0, m_VAO, m_VBO, m_EBO, m_Update);
		return;
	}
	else if (lines != NULL && m_Update == true)
	{
		DrawLinesWithVertexArray(lines, numLines, m_VAO, m_VBO, m_EBO, m_Update);
		m_Update = false;
	}
	else
		ASSERT(true);
}

void DebugRenderer::AddObjects(std::string& name, Point* pts, int numPts)
{
	//m_Renderers[name] = Renderer();
	m_Renderers[name].RenderPoints(pts, numPts);
}

void DebugRenderer::AddObjects(std::string& name, Line* lines, int numLines)
{
	//m_Renderers[name] = Renderer();
	m_Renderers[name].RenderLines(lines, numLines);
}

void DebugRenderer::ShowObject(std::string& name, bool Redraw)
{
	auto it = m_Renderers.find(name);
	if (it != m_Renderers.end())
	{
		auto& renderer = it->second;
		renderer.m_Show = true;
	}
}

void DebugRenderer::HideObject(std::string& name)
{
	auto it = m_Renderers.find(name);
	if (it != m_Renderers.end())
	{
		auto& renderer = it->second;
		renderer.m_Show = false;
	}
}

bool DebugRenderer::IsPresent(std::string& name)
{
	auto it = m_Renderers.find(name);
	if (it != m_Renderers.end())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void DebugRenderer::UpdateObject(std::string& name, void* PtsOrLines, int num)
{
	auto it = m_Renderers.find(name);
	if (it != m_Renderers.end())
	{
		auto& renderer = it->second;
		renderer.m_Show = true;
		renderer.m_Update = true;

		if (renderer.m_Type == Renderer::POINTS)
			renderer.RenderPoints((Point*)PtsOrLines, num);
		else if (renderer.m_Type == Renderer::LINES)
			renderer.RenderLines((Line*)PtsOrLines, num);

	}
	else
		ASSERT(false);
}

void DebugRenderer::DrawAllObjects()
{
	for (auto& Object : m_Renderers)
	{
		auto& renderer = Object.second;
		
		if (renderer.m_Show == false) continue;

		if (renderer.m_Type == Renderer::POINTS)
		{
			renderer.RenderPoints(nullptr, 0);
		}
		else if (renderer.m_Type == Renderer::LINES)
		{
			renderer.RenderLines(nullptr, 0);
		}

	}
}

void DebugRenderer::RemoveObject(std::string name)
{
	m_Renderers.erase(name);
}

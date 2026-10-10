#pragma once
#include "BasicGeomObjects.h"
#include <glad/glad.h>
#include "vector"
#include "unordered_map"
#include "unordered_set"

void GLClearError();
bool GLCheckError();

#define M_PI       3.14159265358979323846
#define M_PI_4     0.785398163397448309616  // pi/4

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLCheckError())



// vertex buffer

class VertexBuffer
{
private:
	unsigned int _ObjectId;
public:
	VertexBuffer() {
		GLCall(glGenBuffers(1, &_ObjectId));
	}
	VertexBuffer(void* data, unsigned int size);
	~VertexBuffer();
	void SetData(void* data, unsigned int size);
	void Bind();
	void UnBind();
};

class IndexBuffer
{
private:
	unsigned int _ObjectId;
	uint32_t _Count;
public:
	IndexBuffer(){
		GLCall(glGenBuffers(1, &_ObjectId));
	};
	IndexBuffer(void* data, unsigned int size);
	~IndexBuffer();
	void SetData(void* data, unsigned int size);
	void Bind();
	void UnBind();
	uint32_t GetCount() { return _Count; }
};

class VertexBufferAttribute
{
public:
	unsigned int type;
	unsigned int count;
	unsigned int normalized;
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferAttribute> m_Attributes;
	unsigned int m_Stride;
public:
	VertexBufferLayout() : m_Stride(0) {};
	template<typename T>
	void Push(unsigned int count)
	{
		static_assert(false);	// this is when type is not matched
	}

	// template specialization
	template<>
	void Push<float>(unsigned int count)
	{
		VertexBufferAttribute attr;
		attr.type = GL_FLOAT;
		attr.count = count;
		attr.normalized = GL_FALSE;
		m_Attributes.push_back(attr);

		m_Stride += sizeof(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		VertexBufferAttribute attr;
		attr.type = GL_UNSIGNED_INT;
		attr.count = count;
		attr.normalized = GL_FALSE;
		m_Attributes.push_back(attr);

		m_Stride += sizeof(GL_UNSIGNED_INT) * count;
	}

	inline unsigned int GetStride() const { return m_Stride; }
	inline auto GetAttributes() const { return m_Attributes; }

};

class VertexArray
{
private:
	unsigned int _ObjectId;
public:
	VertexArray();
	~VertexArray();
	void Bind();
	void UnBind();
	void AddBuffer( VertexBuffer& vb, VertexBufferLayout& layout);
};



void DrawCircle(Point& Pt, float radius, GLuint CircleVBO = 0);
void DrawPoint(Point& Pt, GLuint PointVBO);
void DrawPoints(Point* pts, int numPts, GLuint PointVBO);
void DrawPoints(Point* pts, int numPts, VertexArray& LineVAO, VertexBuffer& LineVBO, IndexBuffer& LineEBO, bool RecomputeBuffer);
void DrawLine(Line& L, GLuint LineVBO);
void DrawLines(Line** L, int numLines, GLuint LineVAO, GLuint LineVBO, GLuint LineEBO, bool RecomputeBuffer);
void DrawLinesWithVertexArray(Line* L, int numLines, VertexArray& LineVAO,
	VertexBuffer& LineVBO, IndexBuffer& LineEBO, bool RecomputeBuffer);


class Renderer
{
public:
	Renderer() : m_Type(NOT_SET), m_Show(true), m_Update(false) {};
	~Renderer()
	{
		cout << "Renderer destructor called" << endl;
	}
	void RenderPoints(Point* pts, int numPts);
	void RenderLines(Line* lines, int numLines);
	enum Type
	{
		POINTS,
		LINES,
		NOT_SET
	};
	Type m_Type;
	bool m_Show;
	bool m_Update;
private:
	VertexArray m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer m_EBO;
};


class DebugRenderer
{
private:
	// Private constructor prevents direct object creation
	DebugRenderer() {}

public:
	// Delete copy operations
	DebugRenderer(const DebugRenderer&) = delete;
	DebugRenderer& operator=(const DebugRenderer&) = delete;

	// Access the single instance
	static DebugRenderer& getInstance() {
		static DebugRenderer instance;   // Created only once
		return instance;
	}

	void AddObjects(std::string& name, Point* pts, int numPts);
	void AddObjects(std::string& name, Line* pts, int numLines);
	void ShowObject(std::string& name, bool Redraw = false);
	void HideObject(std::string& name);
	void DrawAllObjects();
	void UpdateObject(std::string& name, void* ptsOrLines, int numObj);
	void RemoveObject(std::string name);
	bool IsPresent(std::string& name);

private:
	std::unordered_map<std::string, Renderer> m_Renderers;
};
#include "ExampleLayer.h"
using namespace GLCore;
using namespace GLCore::Utils;
#include <chrono>
#include <iostream>

using Clock = std::chrono::steady_clock;

//#include "matplotlibcpp.h"
//
//namespace plt = matplotlibcpp;

#define M_PI       3.14159265358979323846
#define M_PI_4     0.785398163397448309616  // pi/4

// Callback function for printing debug statements
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* msg, const void* data)
{
	const char* _source;
	const char* _type;
	const char* _severity;

	switch (source) {
	case GL_DEBUG_SOURCE_API:
		_source = "API";
		break;

	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		_source = "WINDOW SYSTEM";
		break;

	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		_source = "SHADER COMPILER";
		break;

	case GL_DEBUG_SOURCE_THIRD_PARTY:
		_source = "THIRD PARTY";
		break;

	case GL_DEBUG_SOURCE_APPLICATION:
		_source = "APPLICATION";
		break;

	case GL_DEBUG_SOURCE_OTHER:
		_source = "UNKNOWN";
		break;

	default:
		_source = "UNKNOWN";
		break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		_type = "ERROR";
		break;

	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		_type = "DEPRECATED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		_type = "UDEFINED BEHAVIOR";
		break;

	case GL_DEBUG_TYPE_PORTABILITY:
		_type = "PORTABILITY";
		break;

	case GL_DEBUG_TYPE_PERFORMANCE:
		_type = "PERFORMANCE";
		break;

	case GL_DEBUG_TYPE_OTHER:
		_type = "OTHER";
		break;

	case GL_DEBUG_TYPE_MARKER:
		_type = "MARKER";
		break;

	default:
		_type = "UNKNOWN";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		_severity = "HIGH";
		break;

	case GL_DEBUG_SEVERITY_MEDIUM:
		_severity = "MEDIUM";
		break;

	case GL_DEBUG_SEVERITY_LOW:
		_severity = "LOW";
		break;

	case GL_DEBUG_SEVERITY_NOTIFICATION:
		_severity = "NOTIFICATION";
		break;

	default:
		_severity = "UNKNOWN";
		break;
	}

	printf("%d: %s of %s severity, raised from %s: %s\n",
		id, _type, _severity, _source, msg);
}

ExampleLayer::ExampleLayer()
	: m_CameraController(), 
	m_CoordController(m_CameraController.GetCamera(), 
		Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight())
{
}

ExampleLayer::~ExampleLayer()
{
}

void ExampleLayer::OnAttach()
{

	EnableGLDebugging();

	GLCall(glEnable(GL_DEPTH_TEST));
	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageControl(
		GL_DONT_CARE,
		GL_DONT_CARE,
		GL_DEBUG_SEVERITY_NOTIFICATION,
		0,
		nullptr,
		GL_FALSE
	);
	glDebugMessageCallback(GLDebugMessageCallback, NULL);

	m_Shader = Shader::FromGLSLTextFiles(
		"assets/shaders/test.vert.glsl",
		"assets/shaders/test.frag.glsl"
	);


}

void ExampleLayer::OnDetach()
{
	GLCall(glDeleteVertexArrays(1, &m_AxesVA));
	GLCall(glDeleteBuffers(1, &m_AxesVB));
	GLCall(glDeleteBuffers(1, &m_AxesIB));
}

void ExampleLayer::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);
	m_CoordController.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>(
		[&](MouseButtonPressedEvent& e)
		{
			m_AxesColor = m_AxesAlternateColor;
			return false;
		});
	dispatcher.Dispatch<MouseButtonReleasedEvent>(
		[&](MouseButtonReleasedEvent& e)
		{
			m_AxesColor = m_AxesBaseColor;
			return false;
		});
}



void ExampleLayer::OnUpdate(Timestep ts)
{
	auto FuncStart = Clock::now();
	// clear screen
	GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	GLCall(glUseProgram(m_Shader->GetRendererID()));

	m_CameraController.OnUpdate(ts); // set the camera position and rotation based on user input
	// update the view on viewport, based on updated camera location, by applying VP matrix
	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	// test to modify colour of axis
	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_Color");
	glUniform4fv(location, 1, glm::value_ptr(m_AxesColor));

	GenerateAxes();
	//GenerateConvexHull();
	//GenerateLineIntersection();
	GenerateBZ();

	DebugRenderer& dbgDraw = DebugRenderer::getInstance();
	dbgDraw.DrawAllObjects();

	auto FuncEnd = Clock::now();

	double frameMs =
		std::chrono::duration<double, std::milli>(
			FuncEnd - FuncStart).count();

	//std::cout << "Update Function time: " << frameMs
	//		<< " ms, FPS: "
	//		<< 1000.0 / frameMs << '\n';

		m_UpdateTime = frameMs;



	//auto end = std::chrono::steady_clock::now();

	//double elapsedMs =
	//	std::chrono::duration<double, std::milli>(
	//		end - start
	//	).count();

	//std::cout << "Geometry calculation: "
	//	<< elapsedMs << " ms\n";
}

void ExampleLayer::OnImGuiRender()
{
	auto FuncStart = Clock::now();

	ImGui::Begin("Controls");
	if (ImGui::ColorEdit4("Axes Base Color", glm::value_ptr(m_AxesBaseColor)))
		m_AxesColor = m_AxesBaseColor;
	ImGui::ColorEdit4("Axes Alternate Color", glm::value_ptr(m_AxesAlternateColor));
	
	if (ImGui::Button("Add Point"))
	{
		printf("Add Point Button clicked!\n");
		m_bCreatePoint = true;
	}

	if (ImGui::Button("Stop Adding Point"))
	{
		printf("Stop Adding Point Button clicked!\n");
		m_bCreatePoint = false;
	}
	if (m_bCreatePoint)
	{
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (ImGui::IsMouseClicked(0))
			{
				//ImVec2 mousePos = ImGui::GetMousePos();
				std::pair<float, float>  mousePos = Input::GetMousePosition();

				//uint32_t width = Application::Get().GetWindow().GetWidth();
				//uint32_t height = Application::Get().GetWindow().GetHeight();
				//double aspect_ratio = (double)width / (double)height;
				//// getting world co-ords between x-> -1 to 1 and y-> -1 to 1
				//double xcoord = -aspect_ratio + (mousePos.first / width) * (2.0 * aspect_ratio);
				//double ycoord = 1.0 - (mousePos.second / height) * 2.0;
				glm::vec2 worldCoord = m_CoordController.ScreenToWorld({ mousePos.first, mousePos.second });
				Point pt(worldCoord.x, worldCoord.y);
				//m_BZ.AddControlPoint(Point(worldCoord.x, worldCoord.y));
				m_BZ.AddControlPoint(pt);
				m_BZ.Draw(true);
				m_bCreatePoint = false;
				cout << "Point added: " << worldCoord.x << ", " << worldCoord.y << endl;;

			}
		}
	}

	static bool bGetClosestPoint = false;
	if (ImGui::Button("GetClosestPoint"))
	{
		bGetClosestPoint = true;
		printf("GetClosestPoint clicked! , %d\n", bGetClosestPoint);
	}

	static bool plotGraph = false;
	if (ImGui::Button("Plot"))
		plotGraph = !plotGraph;

	if(plotGraph)
	{
		//std::vector<double> x;
		//std::vector<double> y;

		//for (double t = 0; t <= 10; t += 1)
		//{
		//	x.push_back(t);
		//	y.push_back(std::sin(t));
		//}

		float step = 0.01;
		Point pt;
		Vector vec;
		vector<Line> listOfLines;
		vector<double> param, x, x_dot, x_dot_dot;
		for (double t = 0; t < 1; t = t + step)
		{
			param.push_back(t);

			pt = m_BZ.EvaluateAt(t);
			x.push_back(pt.x);

			vec = m_BZ.FirstDerivative(t);
			x_dot.push_back(vec._x);

			vec = m_BZ.SecondDerivative(t);
			x_dot_dot.push_back(vec._x);
		}

		ImGui::Begin("Berzier");
		if (ImPlot::BeginPlot("Basis Function")) {
			//ImPlot::PlotBars("My Bar Plot", bar_data, 11);
			ImPlot::PlotLine("x", param.data(), x.data(), x.size());
			ImPlot::PlotLine("x_dot", param.data(), x_dot.data(), x.size());
			ImPlot::PlotLine("x_dotdot", param.data(), x_dot_dot.data(), x.size());
			ImPlot::EndPlot();
		}
		ImGui::End();
	}

	if (bGetClosestPoint)
	{
		static bool bSetupDone = false;
		static bool bShowButtons = false;

		if (!ImGui::GetIO().WantCaptureMouse)
		{
			if (ImGui::IsMouseClicked(0) && bSetupDone == false)
			{
				std::pair<float, float>  mousePos = Input::GetMousePosition();
				//uint32_t width = Application::Get().GetWindow().GetWidth();
				//uint32_t height = Application::Get().GetWindow().GetHeight();
				//double aspect_ratio = (double)width / (double)height;
				//// getting world co-ords between x-> -1 to 1 and y-> -1 to 1
				//double xcoord = -aspect_ratio + (mousePos.first / width) * (2.0 * aspect_ratio);
				//double ycoord = 1.0 - (mousePos.second / height) * 2.0;
				glm::vec2 worldCoord = m_CoordController.ScreenToWorld({ mousePos.first, mousePos.second });

				ClosestPointOnCurveOpe.Setup(&m_BZ, Point(worldCoord.x, worldCoord.y), 0);
				bSetupDone = true;
				//ClosestPointResult Result = ClosestPointOnCurveOpe.NextStep();
				//m_ClosestPoint = Result.pt;

				//cout << "Closest Point :" << Result.pt << endl;;
				//bGetClosestPoint = false;
			}
			//DrawPoints(&m_ClosestPoint, 1, ClosestPtPointVAO, ClosestPtPointVBO, ClosestPtPointEBO, true);
		}

		if (bSetupDone == true)
		{
			if (ImGui::Button("GetClosestPoint:NextStep"))
			{
				ClosestPointResult Result = ClosestPointOnCurveOpe.NextStep();
				printf("GetClosestPoint clicked! , %d\n", bGetClosestPoint);
			}
			if (ImGui::Button("GetClosestPoint:Run"))
			{
				ClosestPointResult Result = ClosestPointOnCurveOpe.Run();
				printf("GetClosestPoint clicked! , %d\n", bGetClosestPoint);
				bGetClosestPoint = false;
				DrawPoints(&Result.pt, 1, ClosestPtPointVAO, ClosestPtPointVBO, ClosestPtPointEBO, true);
			}
		}
	}

	static bool bSeeClosestPt = false;
	if (ImGui::Button("See Closest Pt"))
		bSeeClosestPt = !bSeeClosestPt;
	double ClosestPointOnCurveEndMs = 0.0;
	double ClosestPointOnCurveDrawEndMs = 0.0;

	if (bSeeClosestPt)
	{
		static pair<float, float>  lastmousePos = Input::GetMousePosition();
		std::pair<float, float>  mousePos = Input::GetMousePosition();

		if (mousePos.first != lastmousePos.first || mousePos.second != lastmousePos.second)
		{
			auto ClosestPointOnCurveStart = Clock::now();

			lastmousePos = mousePos;

			glm::vec2 worldCoord = m_CoordController.ScreenToWorld({ mousePos.first, mousePos.second });
			ClosestPointOnCurve ClosestPtOnCrv;

			ClosestPtOnCrv.Setup(&m_BZ, Point(worldCoord.x, worldCoord.y), 0);
			ClosestPointResult Result = ClosestPtOnCrv.Run();

			auto ClosestPointOnCurveEnd = Clock::now();

			ClosestPointOnCurveEndMs =
				std::chrono::duration<double, std::milli>(
					ClosestPointOnCurveEnd - ClosestPointOnCurveStart).count();

			//std::cout << "ClosestPointOnCurve Function time: " << ClosestPointOnCurveEndMs
			//	<< " ms, FPS: "
			//	<< 1000.0 / ClosestPointOnCurveEndMs << '\n';

			DebugRenderer& dbgDraw = DebugRenderer::getInstance();
			std::string sPtAtParam("Closest Current Point");
			std::string sLineWithParam("Closest Current Point Line");
			Line L(Result.pt, Point(worldCoord.x, worldCoord.y));

			if (!dbgDraw.IsPresent(sPtAtParam))
			{
				dbgDraw.AddObjects(sPtAtParam, &Result.pt, 1);
			}
			else
				dbgDraw.UpdateObject(sPtAtParam, &Result.pt, 1);

			if (!dbgDraw.IsPresent(sLineWithParam))
			{
				dbgDraw.AddObjects(sLineWithParam, &L, 1);
			}
			else
				dbgDraw.UpdateObject(sLineWithParam, &L, 1);

			auto ClosestPointOnCurveDrawEnd = Clock::now();

			ClosestPointOnCurveDrawEndMs =
				std::chrono::duration<double, std::milli>(
					ClosestPointOnCurveDrawEnd - ClosestPointOnCurveEnd).count();
		}

	}
	if (ImGui::Button("Plot Bezier Functions"))
	{
		m_BZ.Plot();
	}

	static float param = 0.0f;
	ImGui::SliderFloat("Param", &param, 0.0f, 1.0f);
	DebugRenderer& dbgDraw = DebugRenderer::getInstance();
	std::string sPtAtParam("Bezier Point At Param");
	Point initPt = m_BZ.EvaluateAt(param);
	dbgDraw.UpdateObject(sPtAtParam, &initPt, 1);

	auto FuncEnd = Clock::now();

	double frameMs =
		std::chrono::duration<double, std::milli>(
			FuncEnd - FuncStart).count();

	//std::cout << "ImGuiRender Function time: " << frameMs
	//	<< " ms, FPS: "
	//	<< 1000.0 / frameMs << '\n';

	m_ImGuiRenderTime = frameMs;


	ImGui::Begin("Layer Time");
	ImGui::Value("Update Time (ms)", (float)(m_UpdateTime));
	ImGui::Value("ClosestPointOnCurveEndMs (ms)", (float)(ClosestPointOnCurveEndMs));
	ImGui::Value("ClosestPointOnCurveDrawEndMs (ms)", (float)(ClosestPointOnCurveDrawEndMs));
	ImGui::Value("ImGuiRender (ms)", (float)(m_ImGuiRenderTime));
	ImGui::Value("Total Time (ms)", (float)(m_UpdateTime + ClosestPointOnCurveEndMs + ClosestPointOnCurveDrawEndMs + m_ImGuiRenderTime));
	ImGui::End();

	ImGui::End();

}

void ExampleLayer::GenerateAxes()
{
	static bool bAxesComputed = false;
	if (false == bAxesComputed)
	{
		GLCall(glCreateVertexArrays(1, &m_AxesVA));
		GLCall(glBindVertexArray(m_AxesVA));

		float vertices[] = {
			-0.5f,  0.0f, 0.0f,
			 0.5f,  0.0f, 0.0f,
			 0.0f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f
		};

		GLCall(glCreateBuffers(1, &m_AxesVB));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_AxesVB));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));

		uint32_t indices[] = { 0, 1, 2, 3};
		GLCall(glCreateBuffers(1, &m_AxesIB));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_AxesIB));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
		bAxesComputed = true;
	}
	GLCall(glBindVertexArray(m_AxesVA));
	GLCall(glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, nullptr));
	GLCall(glBindVertexArray(0));

}

void ExampleLayer::GenerateConvexHull()
{
	static bool bConvexHullComputed = false;
	if (false == bConvexHullComputed)
	{
		vector<Point> InputPoints;
		InputPoints.push_back(Point{ 1.0, 0.2 });
		InputPoints.push_back(Point{ 0.5, 0.2 });
		InputPoints.push_back(Point{ 0.3, 0.8 });
		InputPoints.push_back(Point{ 0.1, 0.4 });
		InputPoints.push_back(Point{ 0.8, 0.1 });
		InputPoints.push_back(Point{ 0.1, 0.1 });
		InputPoints.push_back(Point{ 0.9, 0.9 });
		InputPoints.push_back(Point{ 0.5, 0.3 });
		InputPoints.push_back(Point{ 0.2, 0.7 });
		InputPoints.push_back(Point{ 0.9, 0.1 });
		InputPoints.push_back(Point{ 0.7, 0.4 });
		InputPoints.push_back(Point{ -0.7, 0.4 });
		InputPoints.push_back(Point{ 0.7, -0.4 });
		m_ConvexHull.SetInputPoints(InputPoints);
		m_ConvexHull.ComputeConvexHull();
		bConvexHullComputed = true;
	}
	m_ConvexHull.Draw();
}

int ExampleLayer::GenerateBZ()
{
	static bool once = true;
	if (once)
	{
		BezierCurve::TestBerizer();
		cout << "Generating Bezier Curve" << endl;
		vector<Point> InputPoints;
		InputPoints.push_back(Point{ 107, 42 });
		InputPoints.push_back(Point{ 57, 22 });
		InputPoints.push_back(Point{ -37, 82 });
		//InputPoints.push_back(Point{ 17, 42 });
		//InputPoints.push_back(Point{ -87, -82 });
		//InputPoints.push_back(Point{ 17, 12 });
		//InputPoints.push_back(Point{ 97, 92 });
		//InputPoints.push_back(Point{ 57, -32 });
		//InputPoints.push_back(Point{ 0.2, 0.7 });
		//InputPoints.push_back(Point{ 0.9, 0.1 });
		//InputPoints.push_back(Point{ 0.7, 0.4 });
		m_BZ.SetControlPoints(InputPoints);
		m_BZ.Draw(true);

		once = false;

	}
	else
		m_BZ.Draw();
	
	std::vector<double> x;
	std::vector<double> y;

	for (double t = 0; t <= 10; t += 0.1)
	{
		x.push_back(t);
		y.push_back(std::sin(t));
	}

	//plt::plot(x, y);
	//plt::show();
	//int   bar_data[11] = ...;
	//float x_data[1000] = ...;
	//float y_data[1000] = ...;

	//ImGui::Begin("My Window");
	//if (ImPlot::BeginPlot("My Plot")) {
	//	//ImPlot::PlotBars("My Bar Plot", bar_data, 11);
	//	ImPlot::PlotLine("My Line Plot", x.data(), y.data(), 1000);
	//	ImPlot::EndPlot();
	//}
	
	DebugRenderer& dbgDraw = DebugRenderer::getInstance();
	std::string sPtAtParam("Bezier Point At Param");
	Point initPt = m_BZ.EvaluateAt(0.0);
	dbgDraw.AddObjects(sPtAtParam, &initPt, 1);

	return 1;
}

void ExampleLayer::GenerateLineIntersection()
{
	//TestLineIntersection();
}
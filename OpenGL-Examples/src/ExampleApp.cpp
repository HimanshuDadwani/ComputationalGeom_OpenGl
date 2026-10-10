#include "GLCore.h"
#include "ExampleLayer.h"
#include "PendulumLayer.h"

using namespace GLCore;

class Example : public Application
{
public:
	Example()
		: Application("OpenGL Examples")
	{
		PushLayer(new ExampleLayer());
		//PushLayer(new PendulumLayer());
	}
};

int main()
{
	std::unique_ptr<Example> app = std::make_unique<Example>();
	app->Run();
}
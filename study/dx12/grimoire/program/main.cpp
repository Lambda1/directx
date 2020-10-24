#include <Windows.h>

#include "./Application.hpp"

int main(int argc, char *argv[])
{
	auto& app = Application::Instance();

	app.Init();

	app.Run();

	app.Terminate();

	return 0;
}
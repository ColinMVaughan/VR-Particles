#include "Application.h"

int main(int argc, char *argv[])
{
	Application App;
	App.Initalize();


	App.RunMainLoop();
	App.Shutdown();

	return 0;
}
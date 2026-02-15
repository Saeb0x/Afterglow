#pragma once

#include "Application.h"

int main(int argc, char** argv)
{
	auto* app = Afterglow::CreateApplication();
	app->Run();

	delete app;
	return 0;
}
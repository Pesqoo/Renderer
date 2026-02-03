#pragma once
#include <Windows.h>
#include "MainWindow.h"

class Application
{
public:
	Application(HINSTANCE hInstance) : m_hInstance(hInstance) {}

	int Run();

private:
	HINSTANCE m_hInstance;
	MainWindow m_mainWindow;
};

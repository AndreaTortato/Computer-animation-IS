#pragma once

#include "../application.h"
#include <vector>

class LabSelector {
protected:
	Application* mCurrentLab;
	bool mIsRunning;
private:
	LabSelector(const LabSelector&);
	LabSelector& operator=(const LabSelector&);
public:
	LabSelector();
	~LabSelector();

	void ImGui(nk_context* context, float xPos);
	Application* getCurrentLab();

	void init();
	void shutdown();
	bool isRunning();
};
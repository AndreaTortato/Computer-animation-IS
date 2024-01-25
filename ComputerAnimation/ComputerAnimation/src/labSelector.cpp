#define _CRT_SECURE_NO_WARNINGS
#include "labSelector.h"
#include <iostream>

#include "lab1.h"
#include "lab2.h"

LabSelector::LabSelector() {
	mCurrentLab = 0;
	mIsRunning = false;
}

void LabSelector::ImGui(nk_context* context, float xPos) {

	// Select lab
	static int selected = 1;
	static const char* options[] = { "Lab 1", "Lab 2" };
	enum examples { LAB1 = 0, LAB2 = 1 };

	if (nk_begin(context, "Lab settings", nk_rect(5.0f, 5.0f, 300.0f, 70), NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_MINIMIZABLE)) {

		nk_layout_row_static(context, 25, 168, 1);
		if (nk_combo_begin_label(context, options[selected], nk_vec2(nk_widget_width(context), 200))) {
			nk_layout_row_dynamic(context, 25, 1);
			for (int i = 0; i < sizeof(options) / sizeof(*options); ++i)
				if (nk_combo_item_label(context, options[i], NK_TEXT_LEFT))
					selected = i;
			nk_combo_end(context);
		}

		switch (selected) 
		{
		case LAB1:
			if (mCurrentLab == 0 || typeid(*mCurrentLab) != typeid(Lab1)) {
				mCurrentLab = new Lab1();
				mCurrentLab->init();
			}
			break;
		
		case LAB2:
			if (mCurrentLab == 0 || typeid(*mCurrentLab) != typeid(Lab2)) {
				mCurrentLab = new Lab2();
				mCurrentLab->init();
			}
			break;
		}
	}
	nk_end(context);
}

LabSelector::~LabSelector() {
	if (mIsRunning) {
		std::cout << "Error, lab selector is still running";
	}
	if (mCurrentLab != 0) {
		std::cout << "Error lab still exists\n";
	}
}

Application* LabSelector::getCurrentLab() {
	return mCurrentLab;
}

void LabSelector::init() {
	if (mIsRunning) {
		std::cout << "Starting lab selector multiple times, might leak memory and resources\n";
	}
	mIsRunning = true;
	mCurrentLab = 0;
}

void LabSelector::shutdown() {
	if (!mIsRunning) {
		std::cout << "Shutting down inactive lab selector\n";
	}
	if (mCurrentLab != 0) {
		mCurrentLab->shutdown();
		delete mCurrentLab;
	}
	mCurrentLab = 0;
	mIsRunning = false;
}

bool LabSelector::isRunning() {
	return mIsRunning;
}
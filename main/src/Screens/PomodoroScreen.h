#pragma once

#include "LV_Interface/LVScreen.h"
#include "Util/Events.h"

class PomodoroTimer;

class PomodoroScreen : public LVScreen {
public:
	PomodoroScreen();

private:
	PomodoroTimer& timer;
	EventQueue events;
	lv_obj_t* phase = nullptr;
	lv_obj_t* remaining = nullptr;
	lv_obj_t* action = nullptr;
	uint32_t lastSecond = UINT32_MAX;

	void onStart() override;
	void onStop() override;
	void loop() override;
	void refresh();
};

#include "PomodoroScreen.h"

#include "Devices/Input.h"
#include "Screens/MainMenu/MainMenu.h"
#include "Services/PomodoroTimer.h"
#include "Theme/theme.h"
#include "Util/Services.h"

#include <cstdio>

PomodoroScreen::PomodoroScreen() :
		timer(*static_cast<PomodoroTimer*>(Services.get(Service::Pomodoro))), events(6){
	lv_obj_set_size(*this, 128, 128);
	lv_obj_set_style_bg_color(*this, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(*this, LV_OPA_COVER, 0);

	phase = lv_label_create(*this);
	lv_obj_set_width(phase, 128);
	lv_obj_set_pos(phase, 0, 13);
	lv_obj_set_style_text_align(phase, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(phase, &devin, 0);
	lv_obj_set_style_text_color(phase, lv_color_white(), 0);

	remaining = lv_label_create(*this);
	lv_obj_set_width(remaining, 128);
	lv_obj_set_pos(remaining, 0, 39);
	lv_obj_set_style_text_align(remaining, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(remaining, &clockfont, 0);
	lv_obj_set_style_text_color(remaining, lv_color_white(), 0);

	action = lv_label_create(*this);
	lv_obj_set_width(action, 124);
	lv_obj_set_pos(action, 2, 92);
	lv_obj_set_style_text_align(action, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_font(action, &devin, 0);
	lv_obj_set_style_text_color(action, lv_color_white(), 0);

	refresh();
}

void PomodoroScreen::onStart(){
	Events::listen(Facility::Input, &events);
	Events::listen(Facility::Pomodoro, &events);
	refresh();
}

void PomodoroScreen::onStop(){
	Events::unlisten(&events);
}

void PomodoroScreen::loop(){
	Event event{};
	while(events.get(event, 0)){
		if(event.facility == Facility::Input){
			auto* input = static_cast<Input::Data*>(event.data);
			if(input->action == Input::Data::Press){
				if(input->btn == Input::Select) timer.toggle();
				else if(input->btn == Input::Up) timer.skip();
				else if(input->btn == Input::Down) timer.reset();
				else if(input->btn == Input::Alt){
					free(event.data);
					transition([](){ return std::make_unique<MainMenu>(); });
					return;
				}
			}
		}
		free(event.data);
		refresh();
	}

	const uint32_t seconds = timer.getRemainingSeconds();
	if(seconds != lastSecond) refresh();
}

void PomodoroScreen::refresh(){
	const uint32_t seconds = timer.getRemainingSeconds();
	lastSecond = seconds;
	lv_label_set_text(phase, timer.getPhase() == PomodoroTimer::Phase::Focus ? "FOCUS" : "BREAK");
	char text[16];
	std::snprintf(text, sizeof(text), "%02lu:%02lu", seconds / 60, seconds % 60);
	lv_label_set_text(remaining, text);

	const char* state = timer.getState() == PomodoroTimer::State::Running ? "SELECT pause" : "SELECT start";
	char help[96];
	std::snprintf(help, sizeof(help), "%s\nUP skip  DOWN reset\nALT back", state);
	lv_label_set_text(action, help);
}

#include "BluetoothDiagnosticsScreen.h"

#include <cstdio>
#include <cstring>

#include "Devices/Input.h"
#include "Notifs/Phone.h"
#include "Screens/Settings/SettingsScreen.h"
#include "Theme/theme.h"
#include "Util/BLEDiagnostics.h"
#include "Util/Services.h"

BluetoothDiagnosticsScreen::BluetoothDiagnosticsScreen() :
		phone(*static_cast<Phone*>(Services.get(Service::Phone))), queue(6){
	lv_obj_set_size(*this, 128, 128);
	lv_obj_set_style_bg_color(*this, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(*this, LV_OPA_COVER, 0);

	auto title = lv_label_create(*this);
	lv_label_set_text(title, "BLUETOOTH DIAGNOSTICS");
	lv_obj_set_pos(title, 3, 2);
	lv_obj_set_style_text_font(title, &devin, 0);
	lv_obj_set_style_text_color(title, lv_color_white(), 0);

	connection = lv_label_create(*this);
	lv_obj_set_pos(connection, 3, 16);
	lv_obj_set_style_text_font(connection, &devin, 0);
	lv_obj_set_style_text_color(connection, lv_color_white(), 0);

	history = lv_label_create(*this);
	lv_obj_set_pos(history, 3, 29);
	lv_obj_set_width(history, 122);
	lv_label_set_long_mode(history, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_font(history, &devin, 0);
	lv_obj_set_style_text_color(history, lv_color_white(), 0);

	auto help = lv_label_create(*this);
	lv_label_set_text(help, "SELECT refresh  ALT back");
	lv_obj_set_pos(help, 3, 117);
	lv_obj_set_style_text_font(help, &devin, 0);
	lv_obj_set_style_text_color(help, lv_color_white(), 0);

	refresh();
}

void BluetoothDiagnosticsScreen::onStart(){
	Events::listen(Facility::Input, &queue);
	Events::listen(Facility::Phone, &queue);
	refresh();
}

void BluetoothDiagnosticsScreen::onStop(){
	Events::unlisten(&queue);
}

void BluetoothDiagnosticsScreen::loop(){
	Event event{};
	while(queue.get(event, 0)){
		if(event.facility == Facility::Input){
			auto input = static_cast<Input::Data*>(event.data);
			if(input->action == Input::Data::Press && input->btn == Input::Alt){
				free(event.data);
				transition([](){ return std::make_unique<SettingsScreen>(); });
				return;
			}
			if(input->action == Input::Data::Press && input->btn == Input::Select){
				refresh();
			}
		}else if(event.facility == Facility::Phone){
			refresh();
		}
		free(event.data);
	}
}

void BluetoothDiagnosticsScreen::refresh(){
	lv_label_set_text(connection, phone.isConnected() ? "iPhone: connected" : "iPhone: disconnected");

	BLEDiagnostics::RecentLine lines[5]{};
	const size_t count = BLEDiagnostics::getRecentLines(lines, 5);
	char text[240]{};
	for(size_t i = 0; i < count; ++i){
		std::strncat(text, lines[i].text, sizeof(text) - std::strlen(text) - 1);
		if(i + 1 < count){
			std::strncat(text, "\n", sizeof(text) - std::strlen(text) - 1);
		}
	}
	if(count == 0){
		std::strcpy(text, "No Bluetooth events yet");
	}
	lv_label_set_text(history, text);
}

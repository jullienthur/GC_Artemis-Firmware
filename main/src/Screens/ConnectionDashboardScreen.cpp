#include "ConnectionDashboardScreen.h"

#include "Devices/Input.h"
#include "Notifs/Phone.h"
#include "Screens/MainMenu/MainMenu.h"
#include "Services/AutoWiFi.h"
#include "Theme/theme.h"
#include "Util/BLEDiagnostics.h"
#include "Util/Services.h"

#include <cstdio>

ConnectionDashboardScreen::ConnectionDashboardScreen() :
		phone(*static_cast<Phone*>(Services.get(Service::Phone))),
		wifi(*static_cast<AutoWiFi*>(Services.get(Service::WiFi))), events(6){
	lv_obj_set_size(*this, 128, 128);
	lv_obj_set_style_bg_color(*this, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(*this, LV_OPA_COVER, 0);

	auto title = lv_label_create(*this);
	lv_label_set_text(title, "CONNECTIONS");
	lv_obj_set_pos(title, 3, 2);
	lv_obj_set_style_text_font(title, &devin, 0);
	lv_obj_set_style_text_color(title, lv_color_white(), 0);

	status = lv_label_create(*this);
	lv_obj_set_pos(status, 3, 18);
	lv_obj_set_width(status, 122);
	lv_obj_set_style_text_font(status, &devin, 0);
	lv_obj_set_style_text_color(status, lv_color_white(), 0);

	recent = lv_label_create(*this);
	lv_obj_set_pos(recent, 3, 60);
	lv_obj_set_width(recent, 122);
	lv_label_set_long_mode(recent, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_font(recent, &devin, 0);
	lv_obj_set_style_text_color(recent, lv_color_white(), 0);

	auto help = lv_label_create(*this);
	lv_label_set_text(help, "SELECT refresh  ALT back");
	lv_obj_set_pos(help, 3, 117);
	lv_obj_set_style_text_font(help, &devin, 0);
	lv_obj_set_style_text_color(help, lv_color_white(), 0);

	refresh();
}

void ConnectionDashboardScreen::onStart(){
	Events::listen(Facility::Input, &events);
	Events::listen(Facility::Phone, &events);
	refresh();
}

void ConnectionDashboardScreen::onStop(){
	Events::unlisten(&events);
}

void ConnectionDashboardScreen::loop(){
	Event event{};
	while(events.get(event, 0)){
		if(event.facility == Facility::Input){
			auto* input = static_cast<Input::Data*>(event.data);
			if(input->action == Input::Data::Press && input->btn == Input::Alt){
				free(event.data);
				transition([](){ return std::make_unique<MainMenu>(); });
				return;
			}
		}
		free(event.data);
		refresh();
	}
}

void ConnectionDashboardScreen::refresh(){
	const char* wifiStatus = "connecting";
	if(wifi.getState() == AutoWiFi::State::Connected) wifiStatus = "connected";
	else if(wifi.getState() == AutoWiFi::State::Disabled) wifiStatus = "off";

	char text[128];
	const std::string ip = wifi.getIP();
	std::snprintf(text, sizeof(text), "iPhone: %s\nWi-Fi: %s%s%s",
		phone.isConnected() ? "connected" : "disconnected", wifiStatus,
		ip.empty() ? "" : "\nIP: ", ip.empty() ? "" : ip.c_str());
	lv_label_set_text(status, text);

	BLEDiagnostics::RecentLine line{};
	if(BLEDiagnostics::getRecentLines(&line, 1) == 1){
		char latest[112];
		std::snprintf(latest, sizeof(latest), "Last Bluetooth event:\n%s", line.text);
		lv_label_set_text(recent, latest);
	}else{
		lv_label_set_text(recent, "No Bluetooth events yet");
	}
}

#pragma once

#include "LV_Interface/LVScreen.h"
#include "Util/Events.h"

class Phone;
class AutoWiFi;

class ConnectionDashboardScreen : public LVScreen {
public:
	ConnectionDashboardScreen();

private:
	Phone& phone;
	AutoWiFi& wifi;
	EventQueue events;
	lv_obj_t* status = nullptr;
	lv_obj_t* recent = nullptr;

	void onStart() override;
	void onStop() override;
	void loop() override;
	void refresh();
};

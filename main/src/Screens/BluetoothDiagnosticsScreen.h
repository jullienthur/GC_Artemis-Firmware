#ifndef CLOCKSTAR_FIRMWARE_BLUETOOTHDIAGNOSTICSSCREEN_H
#define CLOCKSTAR_FIRMWARE_BLUETOOTHDIAGNOSTICSSCREEN_H

#include "LV_Interface/LVScreen.h"
#include "Util/Events.h"

class Phone;

class BluetoothDiagnosticsScreen : public LVScreen {
public:
	BluetoothDiagnosticsScreen();

private:
	void onStart() override;
	void onStop() override;
	void loop() override;
	void refresh();

	Phone& phone;
	EventQueue queue;
	lv_obj_t* connection = nullptr;
	lv_obj_t* history = nullptr;
};

#endif // CLOCKSTAR_FIRMWARE_BLUETOOTHDIAGNOSTICSSCREEN_H

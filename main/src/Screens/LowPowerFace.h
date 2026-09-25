#ifndef CLOCKSTAR_FIRMWARE_LOWPOWERFACE_H
#define CLOCKSTAR_FIRMWARE_LOWPOWERFACE_H

#include "LV_Interface/LVScreen.h"

class ClockLabelBig;

class LowPowerFace : public LVScreen {
public:
	LowPowerFace();
	void refresh();

private:
	ClockLabelBig* clock = nullptr;
};

#endif // CLOCKSTAR_FIRMWARE_LOWPOWERFACE_H

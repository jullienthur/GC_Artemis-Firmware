#include "LowPowerFace.h"

#include "Theme/theme.h"
#include "UIElements/ClockLabelBig.h"

LowPowerFace::LowPowerFace(){
	lv_obj_set_size(*this, 128, 128);
	lv_obj_set_style_bg_color(*this, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(*this, LV_OPA_COVER, 0);

	clock = new ClockLabelBig(*this);
	lv_obj_center(*clock);
	lv_obj_set_style_opa(*clock, LV_OPA_40, 0);
}

void LowPowerFace::refresh(){
	clock->loop();
}

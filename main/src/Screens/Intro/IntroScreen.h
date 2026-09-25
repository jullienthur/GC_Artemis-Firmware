#ifndef ARTEMIS_FIRMWARE_INTROSCREEN_H
#define ARTEMIS_FIRMWARE_INTROSCREEN_H

#include "LV_Interface/LVScreen.h"
#include "Filepaths.hpp"

class IntroScreen : public LVScreen {
public:
	IntroScreen();
	virtual ~IntroScreen();
private:
	void onStart() override;
	void onStop() override;
	void loop() override;

	static constexpr const char* Cached[] = { "/intro/pothos.bin" };
	static constexpr uint32_t FadeTime = 800;
	static constexpr uint32_t HoldTime = 300;

	lv_obj_t* leaf = nullptr;
	uint64_t startTime = 0;
};


#endif //ARTEMIS_FIRMWARE_INTROSCREEN_H

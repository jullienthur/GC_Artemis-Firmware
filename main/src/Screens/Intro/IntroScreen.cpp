#include "IntroScreen.h"
#include "Util/stdafx.h"
#include "Screens/Lock/LockScreen.h"
#include "LV_Interface/FSLVGL.h"
#include "Filepaths.hpp"
#include "Services/StatusCenter.h"
#include "Util/Services.h"

IntroScreen::IntroScreen(){
	lv_obj_set_size(*this, 128, 128);
	lv_obj_set_style_bg_color(*this, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(*this, LV_OPA_COVER, 0);

	leaf = lv_img_create(*this);
	lv_img_set_src(leaf, File::Intro::Pothos);
	lv_obj_center(leaf);
	lv_obj_set_style_opa(leaf, LV_OPA_TRANSP, 0);

	for(const auto img : Cached){
		FSLVGL::addToCache(img);
	}
}

IntroScreen::~IntroScreen(){
	for(const auto img : Cached){
		FSLVGL::removeFromCache(img);
	}
}

void IntroScreen::onStart(){
	if(StatusCenter* status = (StatusCenter*) Services.get(Service::Status)){
		status->circularBlink();
	}

	startTime = millis();
}

void IntroScreen::onStop(){
	if(StatusCenter* status = (StatusCenter*) Services.get(Service::Status)){
		status->circularBlink();
	}

}

void IntroScreen::loop(){
	const uint64_t elapsed = millis() - startTime;
	if(elapsed < FadeTime){
		lv_obj_set_style_opa(leaf, elapsed * LV_OPA_COVER / FadeTime, 0);
	}else if(elapsed < FadeTime + HoldTime){
		lv_obj_set_style_opa(leaf, LV_OPA_COVER, 0);
	}else if(elapsed < 2 * FadeTime + HoldTime){
		const auto fadeElapsed = elapsed - FadeTime - HoldTime;
		lv_obj_set_style_opa(leaf, LV_OPA_COVER - fadeElapsed * LV_OPA_COVER / FadeTime, 0);
	}else{
		transition([](){ return std::make_unique<LockScreen>(); });
	}
}

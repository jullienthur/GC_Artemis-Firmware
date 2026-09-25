#include "PomodoroTimer.h"

#include "Services/ChirpSystem.h"
#include "Util/Events.h"

#include <algorithm>

PomodoroTimer::PomodoroTimer(ChirpSystem& audio) : audio(audio){
	const esp_timer_create_args_t args = {
		.callback = [](void* arg){ static_cast<PomodoroTimer*>(arg)->finished(); },
		.arg = this,
		.name = "pomodoro"
	};
	ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
}

PomodoroTimer::~PomodoroTimer(){
	esp_timer_stop(timer);
	esp_timer_delete(timer);
}

void PomodoroTimer::toggle(){
	Event::Action action;
	{
		std::lock_guard lock(mutex);
		if(state == State::Running){
			const int64_t us = std::max<int64_t>(0, deadlineUs - esp_timer_get_time());
			remainingSeconds = static_cast<uint32_t>((us + 999999) / 1000000);
			esp_timer_stop(timer);
			state = State::Paused;
			action = Event::Paused;
		}else{
			if(state == State::Idle) remainingSeconds = phaseSeconds();
			startLocked();
			action = Event::Started;
		}
	}
	post(action);
}

void PomodoroTimer::reset(){
	{
		std::lock_guard lock(mutex);
		esp_timer_stop(timer);
		state = State::Idle;
		phase = Phase::Focus;
		remainingSeconds = FocusSeconds;
	}
	post(Event::Reset);
}

void PomodoroTimer::skip(){
	{
		std::lock_guard lock(mutex);
		esp_timer_stop(timer);
		phase = phase == Phase::Focus ? Phase::Break : Phase::Focus;
		remainingSeconds = phaseSeconds();
		state = State::Idle;
	}
	post(Event::Finished);
}

PomodoroTimer::State PomodoroTimer::getState() const{
	std::lock_guard lock(mutex);
	return state;
}

PomodoroTimer::Phase PomodoroTimer::getPhase() const{
	std::lock_guard lock(mutex);
	return phase;
}

uint32_t PomodoroTimer::getRemainingSeconds() const{
	std::lock_guard lock(mutex);
	if(state != State::Running) return remainingSeconds;
	const int64_t us = std::max<int64_t>(0, deadlineUs - esp_timer_get_time());
	return static_cast<uint32_t>((us + 999999) / 1000000);
}

uint32_t PomodoroTimer::phaseSeconds() const{
	return phase == Phase::Focus ? FocusSeconds : BreakSeconds;
}

void PomodoroTimer::startLocked(){
	state = State::Running;
	deadlineUs = esp_timer_get_time() + static_cast<int64_t>(remainingSeconds) * 1000000;
	ESP_ERROR_CHECK(esp_timer_start_once(timer, static_cast<uint64_t>(remainingSeconds) * 1000000));
}

void PomodoroTimer::finished(){
	{
		std::lock_guard lock(mutex);
		phase = phase == Phase::Focus ? Phase::Break : Phase::Focus;
		remainingSeconds = phaseSeconds();
		startLocked();
	}
	audio.play({
		Chirp{ .startFreq = 500, .endFreq = 750, .duration = 180 },
		Chirp{ .startFreq = 0, .endFreq = 0, .duration = 100 },
		Chirp{ .startFreq = 750, .endFreq = 1000, .duration = 250 }
	});
	post(Event::Finished);
}

void PomodoroTimer::post(Event::Action action) const{
	Phase eventPhase;
	{
		std::lock_guard lock(mutex);
		eventPhase = phase;
	}
	Events::post(Facility::Pomodoro, Event{ .action = action, .phase = eventPhase });
}

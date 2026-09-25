#pragma once

#include <cstdint>
#include <mutex>

#include <esp_timer.h>

class ChirpSystem;

class PomodoroTimer {
public:
	enum class State : uint8_t { Idle, Running, Paused };
	enum class Phase : uint8_t { Focus, Break };

	struct Event {
		enum Action : uint8_t { Started, Paused, Finished, Reset } action;
		Phase phase;
	};

	explicit PomodoroTimer(ChirpSystem& audio);
	~PomodoroTimer();

	void toggle();
	void reset();
	void skip();
	State getState() const;
	Phase getPhase() const;
	uint32_t getRemainingSeconds() const;

private:
	static constexpr uint32_t FocusSeconds = 25 * 60;
	static constexpr uint32_t BreakSeconds = 5 * 60;

	ChirpSystem& audio;
	esp_timer_handle_t timer = nullptr;
	mutable std::mutex mutex;
	State state = State::Idle;
	Phase phase = Phase::Focus;
	uint32_t remainingSeconds = FocusSeconds;
	int64_t deadlineUs = 0;

	uint32_t phaseSeconds() const;
	void startLocked();
	void finished();
	void post(Event::Action action) const;
};

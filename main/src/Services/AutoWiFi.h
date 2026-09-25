#pragma once

#include <esp_event.h>
#include <esp_netif.h>
#include <esp_timer.h>

#include <string>

// Always-on station client for the owner's configured phone hotspot. It uses
// DHCP and retries periodically while the hotspot is unavailable.
class AutoWiFi {
public:
	enum class State { Disabled, Connecting, Connected };

	AutoWiFi();
	~AutoWiFi();

	State getState() const;
	std::string getIP() const;

private:
	static constexpr uint64_t RetryIntervalUs = 30ULL * 1000ULL * 1000ULL;

	esp_netif_t* netif = nullptr;
	esp_event_handler_instance_t wifiHandler = nullptr;
	esp_event_handler_instance_t ipHandler = nullptr;
	esp_timer_handle_t retryTimer = nullptr;
	State state = State::Disabled;
	std::string ip;

	void connect();
	void handleWiFiEvent(int32_t id, void* data);
	void handleIPEvent(int32_t id, void* data);
	void scheduleRetry();
};

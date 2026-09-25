#include "AutoWiFi.h"

#include "Config/HotspotCredentials.h"

#include <cstring>

#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>

namespace {
constexpr char TAG[] = "AutoWiFi";
}

AutoWiFi::AutoWiFi(){
	if(!HotspotCredentials::Enabled || HotspotCredentials::SSID[0] == '\0'){
		ESP_LOGW(TAG, "No hotspot configured; Wi-Fi remains off");
		return;
	}

	esp_err_t err = esp_netif_init();
	if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
		ESP_ERROR_CHECK(err);
	}

	err = esp_event_loop_create_default();
	if(err != ESP_OK && err != ESP_ERR_INVALID_STATE){
		ESP_ERROR_CHECK(err);
	}

	netif = esp_netif_create_default_wifi_sta();
	if(!netif){
		ESP_LOGE(TAG, "Could not create Wi-Fi station interface");
		return;
	}

	wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&initConfig));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	wifi_config_t config{};
	strncpy(reinterpret_cast<char*>(config.sta.ssid), HotspotCredentials::SSID, sizeof(config.sta.ssid) - 1);
	strncpy(reinterpret_cast<char*>(config.sta.password), HotspotCredentials::Password, sizeof(config.sta.password) - 1);
	config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	config.sta.pmf_cfg.capable = true;
	config.sta.pmf_cfg.required = false;
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
		[](void* arg, esp_event_base_t, int32_t id, void* data){
			static_cast<AutoWiFi*>(arg)->handleWiFiEvent(id, data);
		}, this, &wifiHandler));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
		[](void* arg, esp_event_base_t, int32_t id, void* data){
			static_cast<AutoWiFi*>(arg)->handleIPEvent(id, data);
		}, this, &ipHandler));

	const esp_timer_create_args_t timerArgs = {
		.callback = [](void* arg){ static_cast<AutoWiFi*>(arg)->connect(); },
		.arg = this,
		.name = "wifi_retry"
	};
	ESP_ERROR_CHECK(esp_timer_create(&timerArgs, &retryTimer));
	ESP_ERROR_CHECK(esp_wifi_start());
	state = State::Connecting;
}

AutoWiFi::~AutoWiFi(){
	if(retryTimer){
		esp_timer_stop(retryTimer);
		esp_timer_delete(retryTimer);
	}
	if(wifiHandler){
		esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiHandler);
	}
	if(ipHandler){
		esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, ipHandler);
	}
}

AutoWiFi::State AutoWiFi::getState() const{
	return state;
}

std::string AutoWiFi::getIP() const{
	return ip;
}

void AutoWiFi::connect(){
	if(state == State::Connected) return;

	state = State::Connecting;
	esp_err_t err = esp_wifi_connect();
	if(err != ESP_OK && err != ESP_ERR_WIFI_CONN){
		ESP_LOGW(TAG, "Connection attempt failed: %s", esp_err_to_name(err));
		scheduleRetry();
	}
}

void AutoWiFi::handleWiFiEvent(int32_t id, void*){
	if(id == WIFI_EVENT_STA_START){
		connect();
	}else if(id == WIFI_EVENT_STA_DISCONNECTED){
		state = State::Connecting;
		ip.clear();
		ESP_LOGI(TAG, "Hotspot unavailable; retrying in 30 seconds");
		scheduleRetry();
	}
}

void AutoWiFi::handleIPEvent(int32_t, void* data){
	auto* event = static_cast<ip_event_got_ip_t*>(data);
	char ipText[IP4ADDR_STRLEN_MAX];
	esp_ip4addr_ntoa(&event->ip_info.ip, ipText, sizeof(ipText));
	ip = ipText;
	state = State::Connected;
	if(retryTimer){
		esp_timer_stop(retryTimer);
	}
	ESP_LOGI(TAG, "Connected to hotspot; IP %s", ip.c_str());
}

void AutoWiFi::scheduleRetry(){
	if(!retryTimer || state == State::Connected) return;
	esp_timer_stop(retryTimer);
	ESP_ERROR_CHECK(esp_timer_start_once(retryTimer, RetryIntervalUs));
}

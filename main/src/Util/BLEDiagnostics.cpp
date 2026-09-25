#include "BLEDiagnostics.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <mutex>
#include <nvs.h>
#include <esp_log.h>
#include <esp_timer.h>

namespace {

constexpr uint32_t Magic = 0x424C4431; // "BLD1"
constexpr size_t HistorySize = 16;
constexpr char Namespace[] = "blediag";
constexpr char Key[] = "history";
static const char* TAG = "BLEDiag";

struct Entry {
	uint32_t sequence;
	uint32_t uptimeSeconds;
	uint8_t event;
	uint16_t detail;
};

struct History {
	uint32_t magic;
	uint32_t nextSequence;
	uint8_t nextIndex;
	std::array<Entry, HistorySize> entries;
};

History history{};
nvs_handle_t handle = 0;
bool ready = false;
std::mutex mutex;
QueueHandle_t recordQueue = nullptr;

struct PendingRecord {
	BLEDiagnostics::Event event;
	uint16_t detail;
};

const char* eventName(BLEDiagnostics::Event event){
	switch(event){
		case BLEDiagnostics::Event::Boot: return "boot";
		case BLEDiagnostics::Event::ServerConnected: return "server-connected";
		case BLEDiagnostics::Event::ServerDisconnected: return "server-disconnected";
		case BLEDiagnostics::Event::ClientOpenFailed: return "client-open-failed";
		case BLEDiagnostics::Event::ClientDisconnected: return "client-disconnected";
		case BLEDiagnostics::Event::AuthFailed: return "auth-failed";
		case BLEDiagnostics::Event::Reset: return "reset";
	}
	return "unknown";
}

const char* resetReasonName(uint16_t reason){
	switch(reason){
		case 0: return "unknown";
		case 1: return "power-on";
		case 2: return "external";
		case 3: return "software";
		case 4: return "panic";
		case 5: return "interrupt-wdt";
		case 6: return "task-wdt";
		case 7: return "watchdog";
		case 8: return "deep-sleep";
		case 9: return "brownout";
		case 10: return "sdio";
		case 11: return "usb";
	}
	return "other";
}

void reset(){
	history = {};
	history.magic = Magic;
}

void save(){
	const auto err = nvs_set_blob(handle, Key, &history, sizeof(history));
	if(err != ESP_OK){
		ESP_LOGW(TAG, "Unable to save Bluetooth history: %d", err);
		return;
	}
	if(const auto commitErr = nvs_commit(handle); commitErr != ESP_OK){
		ESP_LOGW(TAG, "Unable to commit Bluetooth history: %d", commitErr);
	}
}

void recordNow(BLEDiagnostics::Event event, uint16_t detail){
	auto& entry = history.entries[history.nextIndex];
	entry.sequence = ++history.nextSequence;
	entry.uptimeSeconds = static_cast<uint32_t>(esp_timer_get_time() / 1000000ULL);
	entry.event = static_cast<uint8_t>(event);
	entry.detail = detail;
	history.nextIndex = (history.nextIndex + 1) % HistorySize;
	save();
}

void recordWorker(void*){
	PendingRecord pending{};
	while(true){
		if(xQueueReceive(recordQueue, &pending, portMAX_DELAY) == pdTRUE){
			std::lock_guard lock(mutex);
			if(ready) recordNow(pending.event, pending.detail);
		}
	}
}

}

void BLEDiagnostics::begin(){
	{
		std::lock_guard lock(mutex);
		if(ready) return;

		const auto err = nvs_open(Namespace, NVS_READWRITE, &handle);
		if(err != ESP_OK){
			ESP_LOGW(TAG, "Unable to open Bluetooth history: %d", err);
			return;
		}

		size_t size = sizeof(history);
		const auto loadErr = nvs_get_blob(handle, Key, &history, &size);
		if(loadErr != ESP_OK || size != sizeof(history) || history.magic != Magic){
			reset();
			save();
		}

		recordQueue = xQueueCreate(32, sizeof(PendingRecord));
		if(recordQueue == nullptr){
			ESP_LOGW(TAG, "Unable to create Bluetooth history queue");
			return;
		}
		ready = true;
	}

	if(xTaskCreate(recordWorker, "BLEDiag", 3072, nullptr, 4, nullptr) != pdPASS){
		std::lock_guard lock(mutex);
		vQueueDelete(recordQueue);
		recordQueue = nullptr;
		ready = false;
		ESP_LOGW(TAG, "Unable to start Bluetooth history worker");
	}
}

void BLEDiagnostics::record(Event event, uint16_t detail){
	if(!ready || recordQueue == nullptr) return;
	const PendingRecord pending{ event, detail };
	xQueueSend(recordQueue, &pending, 0);
}

void BLEDiagnostics::dump(){
	std::lock_guard lock(mutex);
	if(!ready) return;

	auto entries = history.entries;
	std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b){
		return a.sequence < b.sequence;
	});

	printf("BLEDiag: Persistent Bluetooth history:\n");
	for(const auto& entry : entries){
		if(entry.sequence == 0) continue;
		printf("BLEDiag: #%lu uptime=%lus event=%s detail=0x%04x\n",
				static_cast<unsigned long>(entry.sequence),
				static_cast<unsigned long>(entry.uptimeSeconds),
				eventName(static_cast<Event>(entry.event)), entry.detail);
	}
}

size_t BLEDiagnostics::getRecentLines(RecentLine* lines, size_t capacity){
	std::lock_guard lock(mutex);
	if(!ready || lines == nullptr || capacity == 0) return 0;

	auto entries = history.entries;
	std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b){
		return a.sequence > b.sequence;
	});

	size_t count = 0;
	for(const auto& entry : entries){
		if(entry.sequence == 0 || count == capacity) continue;

		const auto event = static_cast<Event>(entry.event);
		if(event == Event::Reset){
			std::snprintf(lines[count].text, sizeof(lines[count].text), "#%lu reset: %s",
					static_cast<unsigned long>(entry.sequence), resetReasonName(entry.detail));
		}else{
			std::snprintf(lines[count].text, sizeof(lines[count].text), "#%lu %lus %s (0x%04x)",
					static_cast<unsigned long>(entry.sequence),
					static_cast<unsigned long>(entry.uptimeSeconds), eventName(event), entry.detail);
		}
		++count;
	}
	return count;
}

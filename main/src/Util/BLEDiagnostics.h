#ifndef CLOCKSTAR_FIRMWARE_BLEDIAGNOSTICS_H
#define CLOCKSTAR_FIRMWARE_BLEDIAGNOSTICS_H

#include <cstddef>
#include <cstdint>

// Persistent, low-volume Bluetooth diagnostics for field testing without USB.
namespace BLEDiagnostics {

enum class Event : uint8_t {
	Boot,
	ServerConnected,
	ServerDisconnected,
	ClientOpenFailed,
	ClientDisconnected,
	AuthFailed,
	Reset,
};

// Text sized for the watch's diagnostics screen. Entries are returned newest
// first so the most useful evidence fits on the small display.
struct RecentLine {
	char text[48];
};

void begin();
void record(Event event, uint16_t detail = 0);
void dump();
size_t getRecentLines(RecentLine* lines, size_t capacity);

}

#endif // CLOCKSTAR_FIRMWARE_BLEDIAGNOSTICS_H

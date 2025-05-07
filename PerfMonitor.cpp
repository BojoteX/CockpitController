#include "src/Globals.h"     

#if DEBUG_PERFORMANCE
#include "src/PsramConfig.h"
#include <string> 
#include <vector>
#include <unordered_map>
#include "src/PerfMonitor.h"
#include "src/PsramConfig.h"  // Needed for PS_NEW()

#define PERFORMANCE_SNAPSHOT_INTERVAL_MS (PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS * 1000UL)

// ——— Profiling state ———
struct ProfAccum { uint64_t sumUs = 0; uint32_t cnt = 0; };
static std::unordered_map<std::string, unsigned long>* _startTimes = nullptr;
static std::unordered_map<std::string, ProfAccum>*     _accumulators = nullptr;

// ——— Monitoring state ———
static unsigned long _lastReportMs = 0;
static unsigned long _lastLoopUs   = 0;
static uint64_t       _busyUsAccum = 0;

// One-time bad-reset alert guard
static bool _alertShown = false;

// === Helpers ===

inline void perfDebugPrint(const char* msg) {
#if VERBOSE_PERFORMANCE_ONLY
    wifiDebugPrint(msg);
#else
    debugPrint(msg);
#endif
}

inline void perfDebugPrintln(const char* msg) {
#if VERBOSE_PERFORMANCE_ONLY
    wifiDebugPrintln(msg);
#else
    debugPrintln(msg);
#endif
}

inline void perfDebugPrintf(const char* format, ...) {
    char buf[256] PS_ATTR;
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
#if VERBOSE_PERFORMANCE_ONLY
    wifiDebugPrint(buf);
#else
    debugPrint(buf);
#endif
}

// True if this reset should trigger an alert
static bool _isBadReset(esp_reset_reason_t r) {
    return (r == ESP_RST_INT_WDT)   ||
           (r == ESP_RST_TASK_WDT)  ||
           (r == ESP_RST_WDT)       ||
           (r == ESP_RST_BROWNOUT)  ||
           (r == ESP_RST_PANIC);
}

// Convert reset codes to a human string
static const char* _resetReasonToString(esp_reset_reason_t r) {
    switch(r) {
      case ESP_RST_UNKNOWN:   return "Unknown";
      case ESP_RST_POWERON:   return "Power-on";
      case ESP_RST_EXT:       return "External";
      case ESP_RST_SW:        return "Software";
      case ESP_RST_PANIC:     return "Panic";
      case ESP_RST_INT_WDT:   return "IntWatchdog";
      case ESP_RST_TASK_WDT:  return "TaskWatchdog";
      case ESP_RST_WDT:       return "OtherWatchdog";
      case ESP_RST_DEEPSLEEP: return "DeepSleepWake";
      case ESP_RST_BROWNOUT:  return "Brownout";
      case ESP_RST_SDIO:      return "SDIO";
      default:                return "??";
    }
}

// === API ===

void initPerfMonitor() {
    _startTimes = ps_new<std::unordered_map<std::string, unsigned long>>();
    _accumulators = ps_new<std::unordered_map<std::string, ProfAccum>>();

    if (!_alertShown) {
        _alertShown = true;
        auto reason = esp_reset_reason();
        if (_isBadReset(reason)) {
            debugPrintln("\n----- ALERT: Unexpected Reset -----");
            debugPrintf("Last reset cause: %s (%d)\n\n", _resetReasonToString(reason), reason);

            perfMonitorUpdate();

            pinMode(LED_BUILTIN, OUTPUT);
            unsigned long start = millis();
            while (millis() - start < 30000UL) {
                digitalWrite(LED_BUILTIN, HIGH);
                delay(500);
                digitalWrite(LED_BUILTIN, LOW);
                delay(500);
            }
            debugPrintln("\nResuming normal operation...\n");
        }
    }

    _lastReportMs = millis();
    _lastLoopUs   = micros();
}

void beginProfiling(const char* label) {
    if (_startTimes) (*_startTimes)[label] = micros();
}

void endProfiling(const char* label) {
    if (!_startTimes || !_accumulators) return;
    auto it = _startTimes->find(label);
    if (it == _startTimes->end()) return;
    uint32_t elapsed = micros() - it->second;
    _startTimes->erase(it);
    auto &a = (*_accumulators)[label];
    a.sumUs += elapsed;
    a.cnt   += 1;
}

void perfMonitorUpdate() {
    unsigned long nowMs = millis();
    if (nowMs - _lastReportMs < PERFORMANCE_SNAPSHOT_INTERVAL_MS) return;
    _lastReportMs = nowMs;

    perfDebugPrintln("+-------------------- PERFORMANCE SNAPSHOT ----------------------+");
    perfDebugPrintln("🔍  Profiling Averages:");

    struct LabelAvg { std::string label; float avgMs; };
    std::vector<LabelAvg> avgs;
    avgs.reserve(_accumulators ? _accumulators->size() : 0);

    if (_accumulators) {
        for (auto& kv : *_accumulators) {
            const auto& label = kv.first;
            const auto& a     = kv.second;
            float avgUs       = a.cnt ? (a.sumUs / static_cast<float>(a.cnt)) : 0.0f;
            avgs.push_back({ label, avgUs / 1000.0f });
        }
        _accumulators->clear();
    }

    for (auto& e : avgs) {
        perfDebugPrintf("    ∘ %-15s : %6.2f ms\n", e.label.c_str(), e.avgMs);
    }

    perfDebugPrintln("+----------------------------------------------------------------+");
    perfDebugPrintln("🕑  System Status:");

    constexpr float frameMs = 1000.0f / POLLING_RATE_HZ;
    float mainLoopAvgMs = 0.0f;
    for (auto& e : avgs) {
        if (e.label == "Main Loop") {
            mainLoopAvgMs = e.avgMs;
            break;
        }
    }

    float pollLoadPct = (mainLoopAvgMs / frameMs) * 100.0f;
    float headroomMs  = frameMs - mainLoopAvgMs;
    float headroomPct = 100.0f - pollLoadPct;
    float scaleFactor = mainLoopAvgMs > 0.0f ? (frameMs / mainLoopAvgMs) : 0.0f;
    uint64_t uptimeSec = esp_timer_get_time() / 1000000ULL;
    uint32_t mins = uptimeSec / 60;
    uint32_t secs = uptimeSec % 60;
    int cpuMHz = ESP.getCpuFreqMHz();
    const char* rr = _resetReasonToString(esp_reset_reason());

    perfDebugPrintf("    ∘ Poll Load     : %5.1f%% of %.2f ms slot\n", pollLoadPct, frameMs);
    perfDebugPrintf("    ∘ Headroom      : %5.3f ms (%5.1f%%)\n", headroomMs, headroomPct);
    perfDebugPrintf("    ∘ Scale Cap.    : %5.2fx current workload\n", scaleFactor);
    if (mins)
        perfDebugPrintf("    ∘ Uptime        : %lum%02lus\n", mins, secs);
    else
        perfDebugPrintf("    ∘ Uptime        : %4lus\n", secs);
    perfDebugPrintf("    ∘ CPU Frequency : %3d MHz\n", cpuMHz);
    perfDebugPrintf("    ∘ Last Reset    : %s\n", rr);

    perfDebugPrintln("+----------------------------------------------------------------+");

    size_t free_int    = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t largest_int = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    float  frag_int    = free_int
                       ? 100.0f * (1.0f - (float)largest_int / (float)free_int)
                       : 0.0f;
    size_t free_psram    = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_psram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    float  frag_psram    = free_psram
                         ? 100.0f * (1.0f - (float)largest_psram / (float)free_psram)
                         : 0.0f;

    perfDebugPrintln("💾  Memory Fragmentation:");
    perfDebugPrintf("    ∘ Internal SRAM : free %6u KB, largest %6u KB, frag %5.1f%%\n",
                    (unsigned)(free_int/1024), (unsigned)(largest_int/1024), frag_int);
    perfDebugPrintf("    ∘ PSRAM Pool    : free %6u KB, largest %6u KB, frag %5.1f%%\n",
                    (unsigned)(free_psram/1024), (unsigned)(largest_psram/1024), frag_psram);

    perfDebugPrintln("+----------------------------------------------------------------+");

    int txAvail   = Serial.availableForWrite();
    int rxWaiting = Serial.available();

    perfDebugPrintln("📡  USB-CDC Buffer Health:");
    perfDebugPrintf("    ∘ TX Free Slots : %6d bytes\n", txAvail);
    perfDebugPrintf("    ∘ RX Pending    : %6d bytes\n", rxWaiting);

    perfDebugPrintln("+----------------------------------------------------------------+");
}
#endif // DEBUG_PERFORMANCE
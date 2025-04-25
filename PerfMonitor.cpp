#include "src/Globals.h"                  

#if DEBUG_PERFORMANCE
#include "src/PerfMonitor.h"
#include <esp_system.h>                   // esp_reset_reason()
#include <esp_timer.h>                    // esp_timer_get_time()
#include <driver/temperature_sensor.h>    // temperature_sensor_*

// Convert seconds to milliseconds at compile time
#define PERFORMANCE_SNAPSHOT_INTERVAL_MS (PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS * 1000UL)

// ——— Profiling state ———
static std::unordered_map<std::string, unsigned long> _startTimes;
struct ProfAccum { uint64_t sumUs = 0; uint32_t cnt = 0; };
static std::unordered_map<std::string, ProfAccum> _accumulators;

// ——— Monitoring state ———
static unsigned long _lastReportMs = 0;
static unsigned long _lastLoopUs   = 0;
static uint64_t       _busyUsAccum = 0;

// Temperature sensor handle
static temperature_sensor_handle_t _tempHandle = nullptr;

// One-time bad-reset alert guard
static bool _alertShown = false;

// === Helpers ===

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
    // 1) Show alert once on bad resets
    if (!_alertShown) {
        _alertShown = true;
        auto reason = esp_reset_reason();
        if (_isBadReset(reason)) {
            debugPrintln("\n----- ALERT: Unexpected Reset -----");
            debugPrintf("Last reset cause: %s (%d)\n\n",
                        _resetReasonToString(reason), reason);

            // Install & enable temp sensor before snapshot
            temperature_sensor_config_t cfg = { .range_min=10, .range_max=50 };
            temperature_sensor_install(&cfg, &_tempHandle);
            temperature_sensor_enable(_tempHandle);

            // Immediate health snapshot
            perfMonitorUpdate();

            // Flash LED for 30s so user sees it
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

    // 2) Seed 10s window and loop timing
    _lastReportMs = millis();
    _lastLoopUs   = micros();

    // 3) Ensure temp sensor ready for future reads
    if (!_tempHandle) {
        temperature_sensor_config_t cfg = { .range_min=10, .range_max=50 };
        temperature_sensor_install(&cfg, &_tempHandle);
        temperature_sensor_enable(_tempHandle);
    }
}

void beginProfiling(const char* label) {
    _startTimes[label] = micros();
}

void endProfiling(const char* label) {
    auto it = _startTimes.find(label);
    if (it == _startTimes.end()) return;
    uint32_t elapsed = micros() - it->second;
    _startTimes.erase(it);
    auto &a = _accumulators[label];
    a.sumUs += elapsed;
    a.cnt   += 1;
}

void perfMonitorUpdate() {
    // 1) Accumulate busy time
    unsigned long nowUs  = micros();
    unsigned long loopUs = nowUs - _lastLoopUs;
    _lastLoopUs          = nowUs;
    _busyUsAccum        += loopUs;

    // 2) Only once per PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS
    unsigned long nowMs = millis();
    if (nowMs - _lastReportMs < PERFORMANCE_SNAPSHOT_INTERVAL_MS) return;
    unsigned long windowMs = nowMs - _lastReportMs;

    // 3) Compute metrics
    float cpuLoadPct = (_busyUsAccum / (float)(windowMs*1000UL))*100.0f;
    size_t freeHeap  = ESP.getFreeHeap();
    size_t maxAlloc  = ESP.getMaxAllocHeap();
    float fragPct    = freeHeap ? ((freeHeap-maxAlloc)/(float)freeHeap)*100.0f : 0.0f;
    unsigned int freeKB  = freeHeap / 1024;
    unsigned int allocKB = maxAlloc / 1024;

    float tempC = 0.0f;
    temperature_sensor_get_celsius(_tempHandle, &tempC);

    auto reason = esp_reset_reason();
    const char* rr = _resetReasonToString(reason);

    uint64_t uptimeSec = esp_timer_get_time() / 1000000ULL;
    uint32_t mins = uptimeSec/60, secs = uptimeSec%60;

    int cpuMHz = ESP.getCpuFreqMHz();

    // 4) Print the snapshot
    debugPrintln("\n\n---------------------------------------- [PERFORMANCE SNAPSHOT] -----------------------------------");
    if (mins) {
        debugPrintf(
          "Uptime: %lum%02lus | CPU: %.1f%% @%dMHz | Heap free: %uKB - Largest block: %uKB - Frag: %.1f%%\n",
          mins, secs, cpuLoadPct, cpuMHz,
          freeKB, allocKB, fragPct
        );
        debugPrintf(
            "Temp: %.1f°C | Reset: %s\n",
            tempC, rr
        );
    } else {
        debugPrintf(
          "Uptime: %4lus | CPU: %.1f%% @%dMHz | Heap free: %uKB - Largest block: %uKB - Frag: %.1f%%\n",
          secs, cpuLoadPct, cpuMHz,
          freeKB, allocKB, fragPct
        );
        debugPrintf(
          "Temp: %.1f°C | Reset: %s\n",
          tempC, rr
        );
    }

    for (auto &kv : _accumulators) {
        const auto &lbl = kv.first;
        const auto &a   = kv.second;
        float avgMs = a.cnt ? (a.sumUs/(float)a.cnt)/1000.0f : 0.0f;
        debugPrintf("[PROF] %-12s: avg %6.2f ms\n",
                    lbl.c_str(), avgMs);
    }
    _accumulators.clear();

    debugPrintln("---------------------------------------------------------------------------------------------------\n");

    // 5) Reset
    _busyUsAccum  = 0;
    _lastReportMs = nowMs;
}

#endif // DEBUG_PERFORMANCE
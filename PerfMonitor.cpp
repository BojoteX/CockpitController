#include "src/Globals.h"     

#if DEBUG_PERFORMANCE
#include "Arduino.h"
#include <string>
#include "src/PerfMonitor.h"
#include <esp_system.h>                     // esp_reset_reason()
#include <esp_timer.h>                      // esp_timer_get_time()
#include <driver/temperature_sensor.h>      // temperature_sensor_*
#include "esp_pm.h"

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
    char buf[256];
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

void setCpuFrequencyMhz(int freq) {
    esp_pm_config_esp32s2_t pm_config = {
        .max_freq_mhz = (uint32_t)freq,
        .min_freq_mhz = (uint32_t)freq,
        .light_sleep_enable = false
    };

    esp_pm_configure(&pm_config);
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

    // Set our CPU Freq. for debugging (we dont use this for production)
    // setCpuFrequencyMhz(80);  // 80, 160 or 240

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
    unsigned long nowMs = millis();
    if (nowMs - _lastReportMs < PERFORMANCE_SNAPSHOT_INTERVAL_MS) return;
    unsigned long windowMs = nowMs - _lastReportMs;

    perfDebugPrintln("\n\n────────────────────────────────────── [ PERFORMANCE SNAPSHOT ] ──────────────────────────────────────");
    perfDebugPrintln("● [Profiling Averages]");

    float mainLoopAvgMs = 0.0f;

    for (auto it = _accumulators.begin(); it != _accumulators.end(); ) {
        const auto& lbl = it->first;
        const auto& a = it->second;
        float avgMs = a.cnt ? (a.sumUs / (float)a.cnt) / 1000.0f : 0.0f;

        if (lbl == "Main Loop") {
            mainLoopAvgMs = avgMs;
        }

        perfDebugPrintf("  └─ %-16s: avg %6.2f ms\n", lbl.c_str(), avgMs);
        it = _accumulators.erase(it);
    }

    // Calculate CPU load
    constexpr float frameMs = (1000.0f / POLLING_RATE_HZ); // Frame time based on polling rate
    float cpuLoadPct = (mainLoopAvgMs / frameMs) * 100.0f;

    // System Info
    size_t freeHeap  = ESP.getFreeHeap();
    size_t maxAlloc  = ESP.getMaxAllocHeap();
    float fragPct    = freeHeap ? ((freeHeap - maxAlloc) / (float)freeHeap) * 100.0f : 0.0f;
    unsigned int freeKB  = freeHeap / 1024;
    unsigned int allocKB = maxAlloc / 1024;

    float tempC = 0.0f;
    temperature_sensor_get_celsius(_tempHandle, &tempC);

    auto reason = esp_reset_reason();
    const char* rr = _resetReasonToString(reason);

    uint64_t uptimeSec = esp_timer_get_time() / 1000000ULL;
    uint32_t mins = uptimeSec / 60, secs = uptimeSec % 60;
    int cpuMHz = ESP.getCpuFreqMHz();

    perfDebugPrintln("\n● [System Status]");
    if (mins) {
        perfDebugPrintf("  └─ Uptime         : %lum%02lus\n", mins, secs);
    } else {
        perfDebugPrintf("  └─ Uptime         : %4lus\n", secs);
    }
    perfDebugPrintf("  └─ CPU Load       : %.1f%%\n", cpuLoadPct);
    perfDebugPrintf("  └─ CPU Frequency  : %d MHz\n", cpuMHz);
    perfDebugPrintf("  └─ Temperature    : %.1f°C\n", tempC);
    perfDebugPrintf("  └─ Heap Free      : %u KB\n", freeKB);
    perfDebugPrintf("  └─ Largest Block  : %u KB\n", allocKB);
    perfDebugPrintf("  └─ Heap Fragment. : %.1f%%\n", fragPct);
    perfDebugPrintf("  └─ Last Reset     : %s\n", rr);

    _lastReportMs = nowMs;

    perfDebugPrintln("──────────────────────────────────────────────────────────────────────────────────────────────────────\n");
}

#endif // DEBUG_PERFORMANCE
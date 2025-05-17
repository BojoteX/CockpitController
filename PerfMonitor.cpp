// PerfMonitor.cpp

#include "src/Globals.h"     

#if DEBUG_PERFORMANCE
#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif // DEBUG_USE_WIFI

// Check the enum in PerfMonitor.h

ProfAccum perfTable[PERF_LABEL_COUNT];

#define PERFORMANCE_SNAPSHOT_INTERVAL_MS (PERFORMANCE_SNAPSHOT_INTERVAL_SECONDS * 1000UL)

// ——— Monitoring state ———
static unsigned long _lastReportMs = 0;
static unsigned long _lastLoopUs   = 0;
static uint64_t       _busyUsAccum = 0;

// One-time bad-reset alert guard
static bool _alertShown = false;

// === Helpers ===
inline void perfDebugPrint(const char* msg) {
#if VERBOSE_PERFORMANCE_ONLY && DEBUG_USE_WIFI
    wifiDebugPrint(msg);
#else
    debugPrint(msg);
#endif
}

inline void perfDebugPrintln(const char* msg) {
#if VERBOSE_PERFORMANCE_ONLY && DEBUG_USE_WIFI
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
#if VERBOSE_PERFORMANCE_ONLY && DEBUG_USE_WIFI
    wifiDebugPrint(buf);
#else
    debugPrint(buf);
#endif
}

void logHeapStatus(const char* label) {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_8BIT);
    Serial.printf("[HEAP] %s: Free=%u | Largest=%u | Frag=%u%%\n",
        label, (uint32_t)info.total_free_bytes,
        (uint32_t)info.largest_free_block,
        100 - ((info.largest_free_block * 100) / info.total_free_bytes));
}

void logCrashDetailIfAny() {
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_PANIC) {
        debugPrintln("🧠 Backtrace not available — use UART for more detail.");
    }
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
    if (!_alertShown) {
        _alertShown = true;
        auto reason = esp_reset_reason();
        if (_isBadReset(reason)) {
            debugPrintln("\n----- ALERT: Unexpected Reset -----");
            debugPrintf("Last reset cause: %s (%d)\n\n", _resetReasonToString(reason), reason);

            logCrashDetailIfAny();
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

void beginProfiling(PerfLabel label) {
    perfTable[label].startUs = micros();
}

void endProfiling(PerfLabel label) {
    uint32_t elapsed = micros() - perfTable[label].startUs;
    perfTable[label].sumUs += elapsed;
    perfTable[label].cnt   += 1;
}

void perfMonitorUpdate() {
    unsigned long nowMs = millis();
    if (nowMs - _lastReportMs < PERFORMANCE_SNAPSHOT_INTERVAL_MS) return;
    _lastReportMs = nowMs;

    perfDebugPrintln("+-------------------- PERFORMANCE SNAPSHOT ----------------------+");
    perfDebugPrintln("🔍  Profiling Averages:");

    float mainLoopAvgMs = 0.0f;
    float totalLoadMs = 0.0f;

    for (int i = 0; i < PERF_LABEL_COUNT; ++i) {
        const auto& a = perfTable[i];
        float avgMs = a.cnt ? (a.sumUs / (float)a.cnt) / 1000.0f : 0.0f;
        perfDebugPrintf("    ∘ %-15s : %6.2f ms\n", perfLabelNames[i], avgMs);
        if (perfIncludedInLoad[i]) totalLoadMs += avgMs;
        if (i == PERF_MAIN_LOOP) mainLoopAvgMs = avgMs;
    }

    for (int i = 0; i < PERF_LABEL_COUNT; ++i) {
    perfTable[i].sumUs = 0;
    perfTable[i].cnt = 0;
    }

    perfDebugPrintln("+----------------------------------------------------------------+");
    perfDebugPrintln("🕑  System Status:");

    constexpr float frameMs = 1000.0f / POLLING_RATE_HZ;

    float pollLoadPct = (totalLoadMs / frameMs) * 100.0f;
    float headroomMs  = frameMs - totalLoadMs;
    float headroomPct = 100.0f - pollLoadPct;
    float scaleFactor = totalLoadMs > 0.0f ? (frameMs / totalLoadMs) : 0.0f;
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

    int rxWaiting = Serial.available();
    int txAvail   = tud_cdc_write_available();  // NOT Serial.availableForWrite()

    perfDebugPrintln("📡  USB-CDC Buffer Health:");
    perfDebugPrintf("    ∘ TX Free Slots : %6d bytes\n", txAvail);
    perfDebugPrintf("    ∘ RX Pending    : %6d bytes\n", rxWaiting);

    perfDebugPrintln("+----------------------------------------------------------------+");
}
#endif // DEBUG_PERFORMANCE
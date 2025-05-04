#include "src/Globals.h"     

#if DEBUG_PERFORMANCE
#include <string> 
#include <vector>
#include <unordered_map>
#include "src/PerfMonitor.h"

#if VERBOSE_MODE_WIFI_ONLY
#define USE_ANSI 1   
#else
#define USE_ANSI 0  
#endif

#if USE_ANSI
  #define ANSI_CYN "\x1b[1;36m"
  #define ANSI_YEL "\x1b[1;33m"
  #define ANSI_GRN "\x1b[32m"
  #define ANSI_MAG "\x1b[35m"
  #define ANSI_RST "\x1b[0m"

  #define BOX_TOP ANSI_CYN "╔═════════════════════════════ 🚀 PERFORMANCE SNAPSHOT 🚀 ═════════════════════════════╗" ANSI_RST
  #define BOX_DIV ANSI_CYN "╠══════════════════════════════════════════════════════════════════════════════════════╣" ANSI_RST
  #define BOX_BOT ANSI_CYN "╚══════════════════════════════════════════════════════════════════════════════════════╝" ANSI_RST
#else
  #define ANSI_CYN ""
  #define ANSI_YEL ""
  #define ANSI_GRN ""
  #define ANSI_MAG ""
  #define ANSI_RST ""

  #define BOX_TOP "+-------------------- PERFORMANCE SNAPSHOT ----------------------+"
  #define BOX_DIV "+----------------------------------------------------------------+"
  #define BOX_BOT "+----------------------------------------------------------------+"
#endif

// #include <driver/temperature_sensor.h>

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
// static temperature_sensor_handle_t _tempHandle = nullptr;

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
            // temperature_sensor_config_t cfg = { .range_min=10, .range_max=50 };
            // temperature_sensor_install(&cfg, &_tempHandle);
            // temperature_sensor_enable(_tempHandle);

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
    // if (!_tempHandle) {
        // temperature_sensor_config_t cfg = { .range_min=10, .range_max=50 };
        // temperature_sensor_install(&cfg, &_tempHandle);
        // temperature_sensor_enable(_tempHandle);
    // }
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
    _lastReportMs = nowMs;

    // — Box top —
    perfDebugPrintln(BOX_TOP);

    // — Profiling Averages —
    perfDebugPrintln(ANSI_YEL "🔍  Profiling Averages:" ANSI_RST);

    struct LabelAvg { std::string label; float avgMs; };
    std::vector<LabelAvg> avgs;
    avgs.reserve(_accumulators.size());

    // collect and clear
    for (auto& kv : _accumulators) {
        const auto& label = kv.first;
        const auto& a     = kv.second;
        float avgUs       = a.cnt ? (a.sumUs / static_cast<float>(a.cnt)) : 0.0f;
        avgs.push_back({ label, avgUs / 1000.0f });
    }
    _accumulators.clear();

    // print all
    for (auto& e : avgs) {
        perfDebugPrintf("    ∘ %-15s : %s%6.2f ms%s\n",
                        e.label.c_str(),
                        ANSI_GRN, e.avgMs, ANSI_RST);
    }

    // — Divider —
    perfDebugPrintln(BOX_DIV);

    // — System Status —
    perfDebugPrintln(ANSI_YEL "🕑  System Status:" ANSI_RST);
    constexpr float frameMs = 1000.0f / POLLING_RATE_HZ;

    // find Main Loop avg for load calculations
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
    uint32_t mins      = uptimeSec / 60;
    uint32_t secs      = uptimeSec % 60;
    int      cpuMHz    = ESP.getCpuFreqMHz();
    const char* rr     = _resetReasonToString(esp_reset_reason());

    perfDebugPrintf("    ∘ Poll Load     : %s%5.1f%%%s of %s%.2f ms%s slot\n",
                    ANSI_GRN, pollLoadPct, ANSI_RST,
                    ANSI_CYN, frameMs, ANSI_RST);
    perfDebugPrintf("    ∘ Headroom      : %s%5.3f ms%s (%s%5.1f%%%s)\n",
                    ANSI_GRN, headroomMs, ANSI_RST,
                    ANSI_GRN, headroomPct, ANSI_RST);
    perfDebugPrintf("    ∘ Scale Cap.    : %s%5.2fx%s current workload\n",
                    ANSI_GRN, scaleFactor, ANSI_RST);
    if (mins)
        perfDebugPrintf("    ∘ Uptime        : %s%lum%02lus%s\n", ANSI_GRN, mins, secs, ANSI_RST);
    else
        perfDebugPrintf("    ∘ Uptime        : %s%4lus%s\n", ANSI_GRN, secs, ANSI_RST);
    perfDebugPrintf("    ∘ CPU Frequency : %s%3d MHz%s\n", ANSI_GRN, cpuMHz, ANSI_RST);
    perfDebugPrintf("    ∘ Last Reset    : %s%s%s\n", ANSI_MAG, rr, ANSI_RST);

    // — Divider —
    perfDebugPrintln(BOX_DIV);

    // — Memory Fragmentation —
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

    perfDebugPrintln(ANSI_YEL "💾  Memory Fragmentation:" ANSI_RST);
    perfDebugPrintf("    ∘ Internal SRAM : free %s%6u KB%s, largest %s%6u KB%s, frag %s%5.1f%%%s\n",
                    ANSI_GRN, (unsigned)(free_int/1024), ANSI_RST,
                    ANSI_GRN, (unsigned)(largest_int/1024), ANSI_RST,
                    ANSI_YEL, frag_int, ANSI_RST);
    perfDebugPrintf("    ∘ PSRAM Pool    : free %s%6u KB%s, largest %s%6u KB%s, frag %s%5.1f%%%s\n",
                    ANSI_GRN, (unsigned)(free_psram/1024), ANSI_RST,
                    ANSI_GRN, (unsigned)(largest_psram/1024), ANSI_RST,
                    ANSI_GRN, frag_psram, ANSI_RST);

    // — Divider —
    perfDebugPrintln(BOX_DIV);

    // — USB-CDC Buffer Health —
    int txAvail   = Serial.availableForWrite();
    int rxWaiting = Serial.available();

    perfDebugPrintln(ANSI_YEL "📡  USB-CDC Buffer Health:" ANSI_RST);
    perfDebugPrintf("    ∘ TX Free Slots : %s%6d bytes%s\n", ANSI_GRN, txAvail, ANSI_RST);
    perfDebugPrintf("    ∘ RX Pending    : %s%6d bytes%s\n", ANSI_GRN, rxWaiting, ANSI_RST);

    // — Box bottom —
    perfDebugPrintln(BOX_BOT);
}

#endif // DEBUG_PERFORMANCE
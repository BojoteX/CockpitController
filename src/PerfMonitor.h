// PerfMonitor.h

#pragma once

#if DEBUG_PERFORMANCE

const char* const perfLabelNames[] = {
    "[LED] GPIO",
    "[LED] PCA9555",
    "[LED] TM1637",
    "[LED] GN1640",
    "[LED] WS2812",
    "[LED] Unknown Device",
    "[REPLAY] Main Loop",
    "[TASK] HID Reports",
    "[TASK] DcsBios Loop",
    "Main Loop"
};
enum PerfLabel : uint8_t {
    PERF_LED_GPIO,
    PERF_LED_PCA9555,
    PERF_LED_TM1637,
    PERF_LED_GN1640,
    PERF_LED_WS2812,
    PERF_LED_UNKNOWN,
    PERF_REPLAY,
    PERF_HIDREPORTS,
    PERF_DCSBIOS,
    PERF_MAIN_LOOP,
    PERF_LABEL_COUNT,  // 🔒 Sentinel for size enforcement
};
constexpr bool perfIncludedInLoad[PERF_LABEL_COUNT] = {
    false,  	// PERF_LED_GPIO
    false,  	// PERF_LED_PCA9555
    false,  	// PERF_LED_TM1637
    false,  	// PERF_LED_GN1640
    false,  	// PERF_LED_WS2812
    false,  	// PERF_LED_UNKNOWN
    false,  	// PERF_REPLAY
    false,  	// PERF_HIDREPORTS
    false,  	// PERF_DCSBIOS
    true   	// PERF_MAIN_LOOP
};
extern const char* const perfLabelNames[];

static_assert(PERF_LABEL_COUNT == sizeof(perfLabelNames) / sizeof(perfLabelNames[0]), "Mismatch: perfLabelNames[] size");
static_assert(PERF_LABEL_COUNT == sizeof(perfIncludedInLoad) / sizeof(perfIncludedInLoad[0]), "Mismatch: perfIncludedInLoad[] size");

struct ProfAccum {
    uint64_t sumUs;
    uint32_t cnt;
    uint32_t startUs;
};
extern ProfAccum perfTable[PERF_LABEL_COUNT];

void initPerfMonitor();
void beginProfiling(PerfLabel label);
void endProfiling(PerfLabel label);
void perfMonitorUpdate();
void printTaskList();

// static_assert(PERF_LABEL_COUNT == sizeof(perfLabelNames) / sizeof(perfLabelNames[0]), "perfLabelNames size mismatch");
// static_assert(PERF_LABEL_COUNT == sizeof(perfIncludedInLoad) / sizeof(perfIncludedInLoad[0]), "perfIncludedInLoad size mismatch");

#endif // DEBUG_PERFORMANCE
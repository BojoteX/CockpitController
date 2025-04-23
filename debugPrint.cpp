#include "src/DebugPrint.h"
#include "src/Globals.h"  
#include "src/WiFiDebug.h"   
#include "Config.h"   

#if DEBUG_PERFORMANCE
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_system.h"
#include "driver/temperature_sensor.h"
#include <esp32-hal.h>    // for ESP.getFreeHeap()
#include "esp_pm.h"
#include <ctype.h>        // for isdigit()

// max number of distinct blocks we’ll profile
static const uint8_t MAX_PROF_BLOCKS = 8;

struct {
  const char* name;
  uint64_t    accumUs;
  uint32_t    count;
  uint64_t    lastStart;
} _profiles[MAX_PROF_BLOCKS];

static uint8_t _profileCount = 0;

// Snapshot interval
static const uint32_t PERF_INTERVAL_MS = 10000;  // 10 s

// Track intervals and profiling for specific segments (e.g DcsBios loop)
static uint32_t _prevTotalRunTime = 0;
static uint32_t _prevIdleRunTime  = 0;
static uint32_t _lastPerfMs       = 0;
uint64_t _busyTimeAccum           = 0;
uint32_t _busyCount               = 0;

// find or register a block by name
static int8_t _findOrCreateProfile(const char* name) {
  for (int8_t i = 0; i < _profileCount; i++) {
    if (strcmp(_profiles[i].name, name) == 0) return i;
  }
  if (_profileCount < MAX_PROF_BLOCKS) {
    _profiles[_profileCount].name      = name;
    _profiles[_profileCount].accumUs   = 0;
    _profiles[_profileCount].count     = 0;
    _profiles[_profileCount].lastStart = 0;
    return _profileCount++;
  }
  return -1;
}

void beginProfiling(const char* name) {
  int8_t idx = _findOrCreateProfile(name);
  if (idx < 0) return;                // too many blocks registered
  _profiles[idx].lastStart = esp_timer_get_time();
}

void endProfiling(const char* name) {
  int8_t idx = _findOrCreateProfile(name);
  if (idx < 0) return;
  uint64_t now = esp_timer_get_time();
  uint64_t dt  = now - _profiles[idx].lastStart;
  _profiles[idx].accumUs += dt;
  _profiles[idx].count   += 1;
}

void reduceCpuFreq() {
  esp_pm_config_esp32s2_t cfg = {
    .max_freq_mhz = 80,    // Default is 240 MHz
    .min_freq_mhz = 80,
    .light_sleep_enable = false
  };
  esp_pm_configure(&cfg);
}

float getCpuTemp() {
  temperature_sensor_config_t temp_sensor = {
    .range_min = 10,
    .range_max = 50
  };
  temperature_sensor_handle_t handle;
  float temp;

  temperature_sensor_install(&temp_sensor, &handle);
  temperature_sensor_enable(handle);
  temperature_sensor_get_celsius(handle, &temp);
  temperature_sensor_disable(handle);
  temperature_sensor_uninstall(handle);

  return temp;
}

void performanceSetup() {

  // seed our FreeRTOS run‑time baselines
  {
    UBaseType_t  taskCount = uxTaskGetNumberOfTasks();
    TaskStatus_t *tasks    = (TaskStatus_t*) pvPortMalloc(taskCount * sizeof(TaskStatus_t));
    uxTaskGetSystemState(tasks, taskCount, &_prevTotalRunTime);

    for (UBaseType_t i = 0; i < taskCount; i++) {
      if (strcmp(tasks[i].pcTaskName, "IDLE") == 0) {
        _prevIdleRunTime = tasks[i].ulRunTimeCounter;
        break;
      }
    }
    vPortFree(tasks);
  }
  _lastPerfMs = millis();   // also reset your 10s timer here

}

void sendPerformanceSnapshot() {
  // 1) snapshot all tasks’ run‑time counters
  UBaseType_t  taskCount   = uxTaskGetNumberOfTasks();
  TaskStatus_t *tasks      = (TaskStatus_t*) pvPortMalloc(taskCount * sizeof(TaskStatus_t));
  uint32_t      totalRunTime;
  uxTaskGetSystemState(tasks, taskCount, &totalRunTime);

  // 2) pull out the Idle task’s counter
  uint32_t idleRunTime = 0;
  for (UBaseType_t i = 0; i < taskCount; i++) {
    if (strcmp(tasks[i].pcTaskName, "IDLE") == 0) {
      idleRunTime = tasks[i].ulRunTimeCounter;
      break;
    }
  }
  vPortFree(tasks);

  // 3) compute deltas since last call
  uint32_t deltaIdle  = idleRunTime  - _prevIdleRunTime;
  uint32_t deltaTotal = totalRunTime - _prevTotalRunTime;
  _prevIdleRunTime    = idleRunTime;
  _prevTotalRunTime   = totalRunTime;

  // 4) interval CPU load + heap
  float cpuLoad       = 0;
  if (deltaTotal)
    cpuLoad = 100.0f * (1.0f - (float)deltaIdle / (float)deltaTotal);
  size_t heap   = ESP.getFreeHeap();

  // 5) After heap
  float avgBusyMs = (_busyCount > 0)
    ? (_busyTimeAccum / _busyCount) / 1000.0f
    : 0.0f;

  // estimate % of each 10 s spent busy:
  float busyPct = (avgBusyMs * _busyCount) / (PERF_INTERVAL_MS) * 100.0f;

  // reset for next window
  _busyTimeAccum = 0;
  _busyCount     = 0;

  // 6) send main PERF line
  char out[128];
  int n = snprintf(out, sizeof(out),
    "[PERF] CPU(10s): %.1f%%  Heap: %u, CPU Temp: %u °C ",
    cpuLoad, (unsigned)heap, getCpuTemp());

  // 7) send separators and blank lines
  #if DEBUG_USE_WIFI
  sendDebug("");  // blank line before
  sendDebug("----------------------- PERFORMANCE SNAPSHOT ------------------------");
  sendDebug(out); // the main PERF line
  #else
  debugPrintln("");  // blank line before
  debugPrintln("----------------------- PERFORMANCE SNAPSHOT ------------------------");
  debugPrintln(out); // the main PERF line
  #endif

  // 8) dump each named profile in one shot
  for (uint8_t i = 0; i < _profileCount; i++) {
    auto &p = _profiles[i];
    if (p.count == 0) continue;
    float avgMs = (float)p.accumUs / p.count / 1000.0f;
    char buf[64];
    snprintf(buf, sizeof(buf), "[PROF] %-12s: avg %6.2f ms", p.name, avgMs);

    #if DEBUG_USE_WIFI
    sendDebug(buf);
    #else 
    debugPrintln(buf);
    #endif

    // reset counters
    p.accumUs = 0;
    p.count   = 0;
  }

  #if DEBUG_USE_WIFI
  sendDebug("---------------------------------------------------------------------");
  sendDebug("");  // blank line after
  #else
  debugPrintln("---------------------------------------------------------------------");
  debugPrintln("");  // blank line after
  #endif

}

void performanceLoop() {
  uint32_t now = millis();
  if (now - _lastPerfMs >= PERF_INTERVAL_MS) {
    _lastPerfMs = now;
    sendPerformanceSnapshot();
  }
}
#endif DEBUG_PERFORMANCE

// Basic prints
void debugPrint(const char* msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(msg);
}
void debugPrint(const String& msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(msg);
}
void debugPrintln(const char* msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(msg);
}
void debugPrintln(const String& msg) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(msg);
}

// Numeric overloads
void debugPrint(uint8_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(uint16_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(uint32_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}
void debugPrint(int val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.print(val, format);
}

void debugPrintln(uint8_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(uint16_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(uint32_t val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}
void debugPrintln(int val, int format) {
  if (DEBUG || !isModeSelectorDCS()) Serial.println(val, format);
}

// Byte write (special chars)
void debugWrite(uint8_t val) {
  if (DEBUG || !isModeSelectorDCS()) Serial.write(val);
}

// Formatted printf-style (renamed explicitly)
void debugPrintf(const char *format, ...) {
  if (DEBUG || !isModeSelectorDCS()) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.print(buffer);
  }
}

void debugPrintfln(const char *format, ...) {
  if (DEBUG || !isModeSelectorDCS()) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Serial.println(buffer);
  }
}
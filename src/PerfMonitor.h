#pragma once

#if DEBUG_PERFORMANCE

// Initialize the performance monitor (call in setup)
void initPerfMonitor();

// Profile a named section
void beginProfiling(const char* label);
void endProfiling  (const char* label);

// Call each loop; prints one 10 s snapshot when ready
void perfMonitorUpdate();

// Check Heap
void logHeapStatus(const char* label);

#else

// Stubs – zero overhead
inline void initPerfMonitor()                 {}
inline void beginProfiling(const char*)       {}
inline void endProfiling  (const char*)       {}
inline void perfMonitorUpdate()               {}

#endif // DEBUG_PERFORMANCE
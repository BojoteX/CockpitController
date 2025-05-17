// Main .ino

#include "src/Globals.h"
#include "src/HIDManager.h"
#include "src/DCSBIOSBridge.h"

#if DEBUG_USE_WIFI
#include "src/WiFiDebug.h"
#endif

// Load PCA Expander Libs
#include <Wire.h>

// PCA Only panels will be autodetected
bool hasBrain     = false; // Brain Controller PCB (e.g TEK Brain uses PCA9555 0x26 for IRCool) autodetected by our program.
bool hasECM       = false; // Panel uses PCA9555 0x22, it is autodetected by our program
bool hasMasterARM = false; // Panel uses 0x5B, it is autodetected by our program

// -- Panel Modules --
#include "src/LeftAnnunciator.h"
#include "src/RightAnnunciator.h"
#include "src/ECMPanel.h"
#include "src/IRCoolPanel.h"
#include "src/MasterARMPanel.h"

// Force compilation of all CUtils internals (Arduino IDE won't compile .cpps in lib automatically)
#include "lib/CUtils/src/CUtils.cpp"

// Checks mode selector state
bool isModeSelectorDCS() {
  #if HAS_HID_MODE_SELECTOR
  return digitalRead(MODE_SWITCH_PIN) == LOW;
  #else
  return true;
  #endif
}

// For reference only
void measureI2Cspeed(uint8_t deviceAddr) {
  uint32_t t0 = micros();
  Wire.requestFrom((uint8_t)deviceAddr, (uint8_t)2);

  while (Wire.available()) {
    Wire.read();
  }
  uint32_t t1 = micros();
  debugPrintf("I²C at 0x%02X Read Time: %u us\n", deviceAddr, t1 - t0);
}

// Reads panel current state during init
void initializePanels() {
  if (hasRA) RightAnnunciator_init();
  if (hasLA) LeftAnnunciator_init();
  if (hasIR) IRCool_init();
  if (hasECM) ECM_init();
  if (hasMasterARM) MasterARM_init();  

  // Consolidated report for all devices
  HIDManager_commitDeferredReport("All devices");
}

void checkHealth() {
  // --- Internal SRAM (on-chip) ---
  size_t free_int    = heap_caps_get_free_size         (MALLOC_CAP_INTERNAL);
  size_t largest_int = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
  float  frag_int    = free_int
                     ? 100.0f * (1.0f - (float)largest_int / (float)free_int)
                     : 0.0f;

  // --- External PSRAM (if present) ---
  size_t free_psram    = heap_caps_get_free_size         (MALLOC_CAP_SPIRAM);
  size_t largest_psram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
  float  frag_psram    = free_psram
                       ? 100.0f * (1.0f - (float)largest_psram / (float)free_psram)
                       : 0.0f;

  // --- Print it all out ---
  debugPrintf(
    "SRAM free: %6u KB, largest: %6u KB, frag: %5.1f%%\n",
    (unsigned)(free_int    / 1024),
    (unsigned)(largest_int / 1024),
    frag_int
  );

  debugPrintf(
    "PSRAM free: %6u KB, largest: %6u KB, frag: %5.1f%%\n",
    (unsigned)(free_psram    / 1024),
    (unsigned)(largest_psram / 1024),
    frag_psram
  );
}

void setup() {

    // Only do this if we have a selector
    #if HAS_HID_MODE_SELECTOR
    // GPIO Setup
    pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
    #endif

    // First parameter is output to Serial, second one is output to UDP (only use this for overriding output)
    debugSetOutput(debugToSerial, debugToUDP);  

    // Sets standard read resolution and attenuation
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);      

    // Activates during DEBUG mode, useful to get Port/Bit info for PCA Devices
    enablePCA9555Logging(DEBUG);     

    // I2C Initialization
    #if PCA_FAST_MODE
    Wire.setTimeOut(100);
    Wire.begin(SDA_PIN, SCL_PIN, 400000);
    #else
    Wire.setTimeOut(100);
    Wire.begin(SDA_PIN, SCL_PIN, 100000);
    #endif

    // Detect PCA Panels (They are disabled by default)
    scanConnectedPanels();    

    DCSBIOSBridge_setup();  // We get CDC started here    
    HIDManager_setup();     // We get HID started here 

    #if DEBUG_USE_WIFI
    wifi_setup();
    #endif

    // Show PCA Panels we discovered
    debugPrintln("\n=== Cockpit Brain Controller Initialization ===");
    printDiscoveredPanels();

    // Automatic PCA Panel detection (use the i2c scanner to get addresses)
    hasBrain     = panelExists(0x26);
    hasECM       = panelExists(0x22);
    hasMasterARM = panelExists(0x5B);

    if(hasBrain) debugPrintln("Brain Controller detected");
    if(hasECM) debugPrintln("ECM Panel detected");
    if(hasMasterARM) debugPrintln("Master ARM Panel detected");

    // Init PCA Inputs
    debugPrintln("Initializing available PCA9555 Inputs...");
    for (uint8_t i = 0; i < discoveredDeviceCount; ++i) {
        initPCA9555AsInput(discoveredDevices[i].address);
    }

    // Initialize PCA9555 Cached Port States explicitly to OFF (active-low LEDs)
    debugPrintln("Initializing PCA9555 Cached Port States...");
    for (uint8_t i = 0; i < discoveredDeviceCount; ++i) {
        uint8_t addr = discoveredDevices[i].address;

        PCA9555_cachedPortStates[addr][0] = 0xFF;
        PCA9555_cachedPortStates[addr][1] = 0xFF;

        Wire.beginTransmission(addr);
        Wire.write(0x02);
        Wire.write(0xFF);
        Wire.write(0xFF);
        Wire.endTransmission();
    }

    // Just for reference, measures PCA9555 bus speed
    measureI2Cspeed(0x26);

    // Active Panels for LED Initialization
    const char* activePanels[7];
    int panelCount = 0;

    #define ADD_PANEL_IF_ENABLED(flag, name) if (flag) activePanels[panelCount++] = name;

    ADD_PANEL_IF_ENABLED(hasIR, "IR");
    ADD_PANEL_IF_ENABLED(hasLA, "LA");
    ADD_PANEL_IF_ENABLED(hasRA, "RA");
    ADD_PANEL_IF_ENABLED(hasCA, "CA");
    ADD_PANEL_IF_ENABLED(hasLockShoot, "LOCKSHOOT");
    ADD_PANEL_IF_ENABLED(hasECM, "ECM");
    ADD_PANEL_IF_ENABLED(hasMasterARM, "ARM");

    /*
    // Run before initializing Display/LED panels
    if (GN1640_detect(GLOBAL_CLK_PIN, CA_DIO_PIN)) {
        debugPrintln("✅ Caution Advisory detected");
    }
    else {
        debugPrintln("⚠️ Caution Advisory NOT detected");
    }
    */

    // Initializes your LEDs / Displays etc.
    debugPrintln("Initializing LEDs...");
    initializeLEDs(activePanels, panelCount);
    
    // When TEST_LEDS is active device enters a menu selection to test LEDs individually. You activate them via Serial Console
    #if TEST_LEDS
        printLEDMenu();
        handleLEDSelection();
        debugPrintln("Exiting LED selection menu. Continuing execution...");
    #endif 
    
    #if IS_REPLAY
    replayData();
    #endif

    // Shows available mem/heap frag etc.
    checkHealth();

    debugPrintf("Selected mode: %s\n", isModeSelectorDCS() ? "DCS-BIOS" : "HID");
    if(DEBUG) {
        debugPrintln("Device is ready! (DEBUG ENABLED)");
    }
    else {
        debugPrintln("Device is ready!");
    }      

    // Syncronize your active panels states
    debugPrintln("Syncronizing Panel state....");
    initializePanels();

    #if DEBUG_PERFORMANCE
    initPerfMonitor(); // this is used for profiling, see debugPrint for details
    #endif   
  
    // Create tasks
    xTaskCreatePinnedToCore(DcsBiosTask, "DCSBIOS", 4096, NULL, 4, NULL, 0);  
    if (!isModeSelectorDCS()) {
        vTaskDelay(pdMS_TO_TICKS(50));  // 50ms cushion before HID starts
        xTaskCreatePinnedToCore(HIDSenderTask, "HIDReportTx", 4096, NULL, 2, NULL, 0);   
    }
}

void loop() {

    // Performance Profiling using beginProfiling("name") -> endProfiling("name") but only when DEBUG_PERFORMANCE  
    #if DEBUG_PERFORMANCE
    beginProfiling(PERF_MAIN_LOOP);
    #endif
        
    DCSBIOSBridge_loop();
    HIDManager_loop();

    // Shadow buffer implementation for Caution Advisory to guarantee row-coherent updates 
    // Solves problem with matrix scanning and partial row writes for GN1640 displays (e.g Caution Advisory)
    if (hasCA) GN1640_tick();

    // Button sampling for the annunciators
    if (hasLA) LeftAnnunciator_loop();
    if (hasRA) RightAnnunciator_loop();

    // Axis and button sampling for IRCool panel 
    if (hasIR) IRCool_loop();  

    // Axis and button sampling for ECM Panel
    if (hasECM) ECM_loop();

    // Axis and button sampling for Master ARM panel 
    if (hasMasterARM) MasterARM_loop();

    // Only when DEBUG is enabled we scan port/bit/position when the user clicks/flips switches on a PCA9555 detected device 
    if (isPCA9555LoggingEnabled()) {
        if (hasBrain) {
            byte p0, p1;
            readPCA9555(0x26, p0, p1);
        }
        if (hasECM) {
            byte p0, p1;
            readPCA9555(0x22, p0, p1);
        }
        if (hasMasterARM) {
            byte p0, p1;
            readPCA9555(0x5B, p0, p1);
        }
    }

    // All profiling blocks REQUIRE we close them
    #if DEBUG_PERFORMANCE
    endProfiling(PERF_MAIN_LOOP);
    #endif

    // If you are profiling in a self contained block outside of the main loop, use perfMonitorUpdate() to simulate iterations.
    #if DEBUG_PERFORMANCE
    perfMonitorUpdate();
    #endif   
}
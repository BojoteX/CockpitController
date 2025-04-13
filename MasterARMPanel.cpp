#include "src/MasterARMPanel.h"
#include "lib/CUtils/src/CUtils.h"
#include "src/HIDManager.h"
#include "src/debugPrint.h"

// Cache local del estado
static byte prevMasterPort0 = 0xFF;

enum Port0Bits {
  MASTER_ARM_AG     = 0,  // Momentary (LOW = pressed)
  MASTER_ARM_AA     = 1,  // Momentary
  MASTER_ARM_DISCH  = 2,  // Momentary
  MASTER_ARM_SWITCH = 3   // ON = HIGH, OFF = LOW
};

void MasterARM_init() {
  delay(50);  // Asegura que el PCA esté inicializado antes de leer

  byte port0, port1;
  if (readPCA9555(MASTERARM_PCA_ADDR, port0, port1)) {
    prevMasterPort0 = port0;

    // 2-pos switch (OFF / ON)
    HIDManager_setNamedButton(
      bitRead(port0, MASTER_ARM_SWITCH) ? "MASTER_ARM_ON" : "MASTER_ARM_OFF",
      true
    );

    // Momentary buttons: detect state at startup (optional)
    HIDManager_setNamedButton("MASTER_ARM_AG",     true, !bitRead(port0, MASTER_ARM_AG));
    HIDManager_setNamedButton("MASTER_ARM_AA",     true, !bitRead(port0, MASTER_ARM_AA));
    HIDManager_setNamedButton("MASTER_ARM_DISCH",  true, !bitRead(port0, MASTER_ARM_DISCH));

    HIDManager_commitDeferredReport();
  } else {
    debugPrintln("❌ Could not read initial state of MasterArm panel.");
  }
}

void MasterARM_loop() {
  byte port0, port1;
  if (!readPCA9555(MASTERARM_PCA_ADDR, port0, port1)) return;

  // 2-position switch (OFF / ON)
  if (bitRead(prevMasterPort0, MASTER_ARM_SWITCH) != bitRead(port0, MASTER_ARM_SWITCH)) {
    HIDManager_setNamedButton(
      bitRead(port0, MASTER_ARM_SWITCH) ? "MASTER_ARM_ON" : "MASTER_ARM_OFF"
    );
  }

  // Momentary buttons (like JETT_SEL)
  if (bitRead(prevMasterPort0, MASTER_ARM_AG) != bitRead(port0, MASTER_ARM_AG)) {
    HIDManager_setNamedButton("MASTER_ARM_AG", false, !bitRead(port0, MASTER_ARM_AG));
  }

  if (bitRead(prevMasterPort0, MASTER_ARM_AA) != bitRead(port0, MASTER_ARM_AA)) {
    HIDManager_setNamedButton("MASTER_ARM_AA", false, !bitRead(port0, MASTER_ARM_AA));
  }

  if (bitRead(prevMasterPort0, MASTER_ARM_DISCH) != bitRead(port0, MASTER_ARM_DISCH)) {
    HIDManager_setNamedButton("MASTER_ARM_DISCH", false, !bitRead(port0, MASTER_ARM_DISCH));
  }

  prevMasterPort0 = port0;
}
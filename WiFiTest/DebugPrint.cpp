#include <Arduino.h>
#include "DebugPrint.h"
void debugPrint(const char* msg) {
  Serial.print(msg);
}

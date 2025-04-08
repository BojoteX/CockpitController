// ************************************************************
// GPIO Pins (LEDs)
//
//
//
// ************************************************************

// Sets all GPIO LEDs
void GPIO_setAllLEDs(bool state) {
  for (int i = 0; i < panelLEDsCount; i++) {
    if (panelLEDs[i].deviceType == DEVICE_GPIO) {
      pinMode(panelLEDs[i].info.gpioInfo.gpio, OUTPUT);
      digitalWrite(panelLEDs[i].info.gpioInfo.gpio, state ? HIGH : LOW);
    }
  }
}
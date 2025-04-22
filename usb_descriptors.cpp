#include "usb_descriptors.h"

static tusb_desc_device_t myDesc = {
  sizeof(tusb_desc_device_t),
  TUSB_DESC_DEVICE,
  0x0200,
  TUSB_CLASS_MISC,
  MISC_SUBCLASS_COMMON,
  MISC_PROTOCOL_IAD,
  CFG_TUD_ENDPOINT0_SIZE,
  0xCAFE,
  0x18C0,
  0x0100,
  1,
  2,
  3,
  1
};

static const char* myStrings[] = {
  (const char[]){0x09,0x04},
  "Bojote",
  "F/A-18C Hornet Cockpit Controller",
  "FA18C-BC-1"
};
static uint16_t utf16buf[32];

extern "C" {

uint8_t const* tud_descriptor_device_cb(void) {
  return (uint8_t const*)&myDesc;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  if (index >= sizeof(myStrings)/sizeof(myStrings[0])) return nullptr;

  if (index == 0) {
    utf16buf[0] = (TUSB_DESC_STRING << 8) | (2 * 1 + 2);
    utf16buf[1] = 0x0409;
    return utf16buf;
  }

  const char* str = myStrings[index];
  size_t len = strlen(str);
  if (len > 31) len = 31;

  utf16buf[0] = (TUSB_DESC_STRING << 8) | (len * 2 + 2);
  for (size_t i = 0; i < len; i++)
    utf16buf[i + 1] = str[i];

  return utf16buf;
}

}

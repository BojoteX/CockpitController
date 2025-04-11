import json

with open('FA-18C_hornet.json', 'r') as f:
    data = json.load(f)

prefixes = ('SPIN_LT', 'MASTER_MODE_', 'LS_', 'JETT_SEL_LT', 'CLIP_', 'LH_', 'RH_')
led_defs = []

for category, controls in data.items():
    for ctrl_name, ctrl_data in controls.items():
        if ctrl_name.startswith(prefixes) and ctrl_data['control_type'] == 'led':
            output = ctrl_data['outputs'][0]
            address = output['address']
            mask = output['mask']
            shift = output['shift_by']
            led_defs.append((ctrl_name, address, mask, shift))

# Sort by address for binary search
led_defs.sort(key=lambda x: x[1])

# Generate DCSMappings.h
with open('DCSMappings.h', 'w') as header_file:
    header_file.write("// Auto-generated DCS LED Definitions\n")
    header_file.write("#ifndef DCSMAPPINGS_H\n#define DCSMAPPINGS_H\n\n")

    header_file.write("#include <Arduino.h>\n\n")

    header_file.write("struct DcsLed {\n")
    header_file.write("  const char* label;\n")
    header_file.write("  uint16_t address;\n")
    header_file.write("  uint16_t mask;\n")
    header_file.write("  uint8_t shift;\n")
    header_file.write("};\n\n")

    header_file.write(f"#define NUM_DCS_LEDS {len(led_defs)}\n\n")

    header_file.write("const DcsLed dcsLeds[NUM_DCS_LEDS] = {\n")
    for name, addr, mask, shift in led_defs:
        header_file.write(f"  {{\"{name}\", 0x{addr:04X}, 0x{mask:04X}, {shift}}},\n")
    header_file.write("};\n\n")

    # Declare callback functions
    for idx in range(len(led_defs)):
        header_file.write(f"void handleLedCallback{idx}(unsigned int newValue);\n")

    header_file.write("\ntypedef void (*CallbackPtr)(unsigned int);\n")
    header_file.write("extern const CallbackPtr ledCallbacks[NUM_DCS_LEDS];\n\n")

    header_file.write("#endif\n")

# Generate DCSMappings.cpp
with open('DCSMappings.cpp', 'w') as cpp_file:
    cpp_file.write("// Auto-generated DCS LED Callbacks\n")
    cpp_file.write('#include "DCSMappings.h"\n')
    cpp_file.write('#include "../LEDControl.h"\n\n')

    cpp_file.write("extern void handleLedChange(uint16_t address, uint16_t mask, uint8_t shift, unsigned int newValue);\n\n")

    for idx, (name, addr, mask, shift) in enumerate(led_defs):
        cpp_file.write(f"void handleLedCallback{idx}(unsigned int newValue) {{\n")
        cpp_file.write(f"  handleLedChange(0x{addr:04X}, 0x{mask:04X}, {shift}, newValue);\n")
        cpp_file.write("}\n\n")

    cpp_file.write("const CallbackPtr ledCallbacks[NUM_DCS_LEDS] = {\n")
    for idx in range(len(led_defs)):
        cpp_file.write(f"  handleLedCallback{idx},\n")
    cpp_file.write("};\n")

# Generate callbacks.txt for easy Arduino INO integration
with open('callbacks.txt', 'w') as callback_file:
    callback_file.write("// Auto-generated callbacks (copy-paste into main .ino)\n\n")
    for name, addr, mask, shift in led_defs:
        callback_file.write(
            f"void onLEDChange_{name}(unsigned int newValue) {{ setLED(\"{name}\", newValue); }}\n"
        )

    callback_file.write("\n// DCS-BIOS buffer declarations:\n")
    for name, addr, mask, shift in led_defs:
        callback_file.write(
            f"DcsBios::IntegerBuffer {name}_Buffer(0x{addr:04X}, 0x{mask:04X}, {shift}, onLEDChange_{name});\n"
        )

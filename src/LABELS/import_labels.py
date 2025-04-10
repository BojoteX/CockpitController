import json
import re

# Read cockpit-defined ButtonMappings.h
with open('ButtonMappings.h', 'r', encoding='utf-8') as file:
    button_content = file.read()

# Extract cockpit button labels using regex
button_labels = re.findall(r'\{\s*"(.*?)"\s*,', button_content)

# Read cockpit-defined LEDMappings.h
with open('LEDMappings.h', 'r', encoding='utf-8') as file:
    led_content = file.read()

# Extract cockpit LED labels using regex
led_labels = re.findall(r'\{\s*"(.*?)"\s*,\s*DEVICE', led_content)

# Load DCS-BIOS JSON data
with open('FA-18C_hornet.json', 'r', encoding='utf-8') as file:
    hornet_data = json.load(file)

# Prepare header file
header_content = """
#ifndef FA18C_MAPPINGS_H
#define FA18C_MAPPINGS_H

#include <stdint.h>

// ------------------ OUTPUT MAPPING STRUCT ------------------
typedef struct {
  uint16_t address;
  uint16_t mask;
  uint8_t shift;
  const char* ledLabel;
} DcsOutputMapping;

// ------------------ INPUT MAPPING STRUCT WITH VALUE SUPPORT ------------------
typedef struct {
  const char* command;     // DCS-BIOS command
  const char* inputLabel;  // Your firmware label
  const char* onValue;     // Value to send when pressed/on
  const char* offValue;    // Value to send when released/off
} DcsCommand;

// ------------------ OUTPUT MAPPINGS (DCS => LED) ------------------
static const DcsOutputMapping DCSOutputs[] = {
"""

# Cross-reference cockpit LED labels with JSON
for category, controls in hornet_data.items():
    for control_name, details in controls.items():
        identifier = details["identifier"]
        if identifier in led_labels:
            for output in details.get("outputs", []):
                address = output.get("address", 0)
                mask = output.get("mask", 0)
                shift = output.get("shift_by", 0)
                header_content += f"  {{{address}, {mask}, {shift}, \"{identifier}\"}},\n"

header_content += "};\nstatic const size_t numDCSOutputs = sizeof(DCSOutputs) / sizeof(DcsOutputMapping);\n\n"
header_content += "// ------------------ INPUT MAPPINGS (COCKPIT INPUT => DCS COMMAND) ------------------\nstatic const DcsCommand DCSCommands[] = {\n"

# Cross-reference cockpit button labels with JSON
for category, controls in hornet_data.items():
    for control_name, details in controls.items():
        command = details["identifier"]
        if command in button_labels:
            for input_detail in details.get("inputs", []):
                description = input_detail.get("description", "UNKNOWN")
                on_value = "1"  # Default on/off values, adjust as necessary
                off_value = "0"
                header_content += f"  {{\"{command}\", \"{description}\", \"{on_value}\", \"{off_value}\"}},\n"

header_content += "};\nstatic const size_t numDCSInputs = sizeof(DCSCommands) / sizeof(DcsCommand);\n\n#endif // FA18C_MAPPINGS_H\n"

# Write to header file
with open('FA18C_Mappings.h', 'w', encoding='utf-8') as header_file:
    header_file.write(header_content)

print("FA18C_Mappings.h generated successfully!")

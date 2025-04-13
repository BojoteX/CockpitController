import json
import re
from collections import defaultdict

# ------- CONFIGURATION -------
PROCESS_ALL_PANELS = False
json_file = 'FA-18C_hornet.json'
lua_file = 'FA-18C_hornet.lua'
header_output = 'DCSBIOSBridgeData.h'
MAX_ENTRIES = 600

# Panels of interest
target_objects = [
    'Fire Systems', 'Caution Light Panel', 'Dispenser/EMC Panel', 'LH Advisory Panel',
    'Left Engine Fire Warning Extinguisher Light', 'Lock Shoot Lights',
    'Map Gain/Spin Recovery Panel', 'Master Arm Panel', 'Master Caution Light',
    'RH Advisory Panel', 'Right Engine Fire Warning Extinguisher Light',
    'Interior Lights Panel', 'APU Fire Warning Extinguisher Light'
]

# Parse selector value mappings from LUA
selector_value_map = {}
with open(lua_file, "r", encoding="utf-8", errors="ignore") as f:
    lua_content = f.read()
    pattern = re.compile(r'defineTumb\("([^"]+)",\s*\d+,\s*\d+,\s*\d+,\s*[\d.]+,\s*\{[^}]+\}.*?,\s*"[^"]+",\s*"([^"]+)"\)')
    for identifier, description in pattern.findall(lua_content):
        if "," in description:
            try:
                name_part, label_part = description.split(",", 1)
                labels = [v.strip() for v in label_part.split("/")]
                selector_value_map[identifier] = {
                    "name": name_part.strip(),
                    "labels": labels
                }
            except Exception:
                pass

# Extract entries from JSON
entries = []
address_to_indices = defaultdict(list)
with open(json_file, encoding='utf-8') as f:
    data = json.load(f)
    for panel, controls in data.items():
        if not PROCESS_ALL_PANELS and panel not in target_objects:
            continue
        for key, item in controls.items():
            ident = item.get('identifier', key)
            outputs = item.get('outputs', [])
            if not outputs:
                continue
            out = outputs[0]
            if not all(k in out for k in ("address", "mask", "shift_by")):
                continue
            address = out['address']
            mask = out['mask']
            shift = out['shift_by']
            index = len(entries)
            entries.append((address, mask, shift, ident))
            address_to_indices[address].append(index)

# Assign static indices
entries.sort(key=lambda e: e[3])  # Sort by label

# --- Start Output ---
with open(header_output, 'w', encoding='utf-8') as f:
    f.write('// Auto-generated dense DCS-BIOS output key table\n')
    f.write('// Lookup uses raw `if-else` tree for compatibility\n\n')
    f.write('#pragma once\n#include <stdint.h>\n#include <string.h>\n\n')
    f.write('struct DcsOutputKey {\n')
    f.write('    uint16_t address;\n')
    f.write('    uint16_t mask;\n')
    f.write('    uint8_t shift;\n')
    f.write('    const char* label;\n')
    f.write('};\n\n')

    f.write(f'#define NUM_ENTRIES {MAX_ENTRIES}\n\n')

    # Write key table
    f.write('static const DcsOutputKey DcsOutputKeyTable[NUM_ENTRIES] = {\n')
    for i, (addr, mask, shift, label) in enumerate(entries):
        f.write(f'    [{i}] = {{ 0x{addr:04X}, 0x{mask:04X}, {shift}, "{label}" }},\n')
    for i in range(len(entries), MAX_ENTRIES):
        f.write(f'    [{i}] = {{ 0, 0, 0, nullptr }},\n')
    f.write('};\n\n')

    # Lookup function
    f.write('static inline const DcsOutputKey* lookup(const char* label, uint16_t address, uint16_t mask, uint8_t shift) {\n')
    for i, (addr, mask, shift, label) in enumerate(entries):
        f.write(f'    if (strcmp(label, "{label}") == 0 && address == 0x{addr:04X} && mask == 0x{mask:04X} && shift == {shift}) return &DcsOutputKeyTable[{i}];\n')
    f.write('    return nullptr;\n')
    f.write('}\n\n')

    # Address → entry index map
    for address, indices in address_to_indices.items():
        f.write(f'static const uint8_t addr_{address:04X}[] = {{ {", ".join(map(str, indices))} }};\n')

    f.write('\nstruct DcsAddressMap {\n')
    f.write('    uint16_t address;\n')
    f.write('    const uint8_t* indices;\n')
    f.write('    uint8_t count;\n')
    f.write('};\n\n')

    f.write('static const DcsAddressMap AddressMap[] = {\n')
    for address, indices in address_to_indices.items():
        f.write(f'    {{ 0x{address:04X}, addr_{address:04X}, {len(indices)} }},\n')
    f.write('};\n\n')

    # Enhanced selector comments
    f.write('/* ---- Enhanced Selector Info (from .lua) ---- */\n')
    for _, _, _, label in entries:
        if label in selector_value_map:
            desc = selector_value_map[label]
            label_str = ', '.join(f"{i}={v}" for i, v in enumerate(desc["labels"]))
            f.write(f'// {label} ({desc["name"]}): {label_str}\n')

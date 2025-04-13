import json
import re
from pathlib import Path
from collections import defaultdict

# ===== CONFIGURATION =====
PROCESS_ALL_PANELS = False
json_file = 'FA-18C_hornet.json'
lua_file = 'FA-18C_hornet.lua'
header_output = 'DCSBIOSBridgeData.h'

target_objects = [
    'Fire Systems', 'Caution Light Panel', 'Dispenser/EMC Panel', 'LH Advisory Panel',
    'Left Engine Fire Warning Extinguisher Light', 'Lock Shoot Lights',
    'Map Gain/Spin Recovery Panel', 'Master Arm Panel', 'Master Caution Light',
    'RH Advisory Panel', 'Right Engine Fire Warning Extinguisher Light',
    'Interior Lights Panel', 'APU Fire Warning Extinguisher Light'
]

# ===== Parse LUA Descriptions =====
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

# ===== Parse JSON Controls =====
address_entries = defaultdict(list)
selector_info_comments = []

with open(json_file, encoding='utf-8') as f:
    data = json.load(f)
    for panel, controls in data.items():
        if not PROCESS_ALL_PANELS and panel not in target_objects:
            continue
        for key, item in controls.items():
            ident = item.get('identifier', key)
            ctype = item.get('control_type', '').lower()
            outputs = item.get('outputs', [])
            if not outputs:
                continue
            out = outputs[0]
            if "address" not in out or "mask" not in out or "shift_by" not in out:
                continue
            address = out['address']
            mask = out['mask']
            shift = out['shift_by']
            is_selector = ctype == "selector"
            address_entries[address].append((ident, mask, shift, is_selector))
            if ident in selector_value_map:
                labels = selector_value_map[ident]["labels"]
                label_str = ', '.join(f"{i}={label}" for i, label in enumerate(labels))
                selector_info_comments.append(f'// {ident} ({selector_value_map[ident]["name"]}): {label_str}')

# ===== Write Header Output =====
with open(header_output, 'w', encoding='utf-8') as f:
    f.write('// Auto-generated hybrid IntegerBuffer with centralized bit dispatching\n\n')
    f.write('#pragma once\n\n#include <Arduino.h>\n#include <DcsBios.h>\n\n')
    f.write('void onLedChange(const char* label, unsigned int value);\n')
    f.write('void onSelectorChange(const char* label, unsigned int value);\n\n')

    # Shared Dispatch Helper
    f.write('// Shared callback dispatcher\n')
    f.write('inline void handleDcsWrite(const char* label, uint16_t raw, uint16_t mask, uint8_t shift, bool isSelector) {\n')

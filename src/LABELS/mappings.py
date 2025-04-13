import json
import re
from collections import defaultdict

# -------- CONFIGURATION --------
PROCESS_ALL_PANELS = False
json_file = "FA-18C_hornet.json"
lua_file = "FA-18C_hornet.lua"
output_header_file = "DCSBIOSBridgeData.h"

target_objects = [
    'Fire Systems', 'Caution Light Panel', 'Dispenser/EMC Panel', 'LH Advisory Panel',
    'Left Engine Fire Warning Extinguisher Light', 'Lock Shoot Lights',
    'Map Gain/Spin Recovery Panel', 'Master Arm Panel', 'Master Caution Light',
    'RH Advisory Panel', 'Right Engine Fire Warning Extinguisher Light',
    'Interior Lights Panel', 'APU Fire Warning Extinguisher Light'
]

def dcs_hash(addr, mask, shift):
    return ((addr << 16) ^ mask ^ (shift << 1)) & 0xFFFFFFFF

# -------- LOAD LUA --------
selector_value_map = {}
with open(lua_file, "r", encoding="utf-8", errors="ignore") as f:
    lua_content = f.read()
    pattern = re.compile(r'defineTumb\("([^"]+)",.*?"([^"]+)"\)')
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

# -------- LOAD JSON --------
with open(json_file, encoding="utf-8") as f:
    data = json.load(f)

output_hashmap = defaultdict(list)
input_entries = []
selector_info_comments = []

for panel, controls in data.items():
    if not PROCESS_ALL_PANELS and panel not in target_objects:
        continue
    for key, item in controls.items():
        ident = item.get('identifier', key)
        ctype = item.get('control_type', '').lower()
        inputs = item.get('inputs', [])
        outputs = item.get('outputs', [])
        desc = item.get('description', '').split(",")[0]

        if ctype == "led" and outputs:
            out = outputs[0]
            if all(k in out for k in ("address", "mask", "shift_by")):
                h = dcs_hash(out['address'], out['mask'], out['shift_by'])
                output_hashmap[h].append(ident)

        if inputs:
            for inp in inputs:
                if inp.get('interface') == 'set_state' and 'max_value' in inp:
                    input_entries.append({
                        'label': ident,
                        'max_value': inp['max_value'],
                        'description': desc
                    })
                    if ident in selector_value_map:
                        label_data = selector_value_map[ident]
                        label_str = ', '.join(f"{i}={v}" for i, v in enumerate(label_data['labels']))
                        selector_info_comments.append(f'// {ident} ({label_data["name"]}): {label_str}')
                    else:
                        if '/' in desc:
                            labels = desc.split('/')[:4]
                            label_str = ', '.join(f"{i}={v.strip()}" for i, v in enumerate(labels))
                            selector_info_comments.append(f'// {ident} ({desc}): {label_str}')
                    break

# -------- GENERATE HEADER --------
with open(output_header_file, "w", encoding="utf-8") as f:
    f.write("// Auto-generated DCSBIOS Data Mapping Header with Multi-label Fast Lookup\n")
    f.write("#pragma once\n\n")
    f.write("#include <stdint.h>\n#include <unordered_map>\n#include <vector>\n\n")

    f.write("inline uint32_t dcsHash(uint16_t addr, uint16_t mask, uint8_t shift) {\n")
    f.write("    return ((uint32_t)addr << 16) ^ (uint32_t)mask ^ ((uint32_t)shift << 1);\n")
    f.write("}\n\n")

    f.write("static const std::unordered_map<uint32_t, std::vector<const char*>> DcsOutputHashTable = {\n")
    for h, labels in output_hashmap.items():
        label_str = ', '.join(f'"{label}"' for label in labels)
        f.write(f'    {{ 0x{h:08X}, {{ {label_str} }} }},\n')
    f.write("};\n\n")

    f.write("struct DcsInputEntry {\n    const char* label;\n    uint8_t max_value;\n    const char* description;\n};\n\n")
    f.write("static const DcsInputEntry DcsInputTable[] = {\n")
    for entry in input_entries:
        label = entry["label"]
        maxval = entry["max_value"]
        desc = entry["description"].replace('"', '\\"')
        f.write(f'    {{ "{label}", {maxval}, "{desc}" }},\n')
    f.write("};\n\n")

    f.write("// ---- Enhanced Selector Info (from LUA) ----\n")
    for comment in selector_info_comments:
        f.write(comment + "\n")

print(f"[✓] Generated {output_header_file} with {len(output_hashmap)} output records and {len(input_entries)} input records.")

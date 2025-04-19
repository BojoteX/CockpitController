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

output_entries = []
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

        if outputs and (ctype == "led" or ctype == "limited_dial"):
            out = outputs[0]
            if all(k in out for k in ("address", "mask", "shift_by")):
                output_entries.append({
                    'label': ident,
                    'addr': out['address'],
                    'mask': out['mask'],
                    'shift': out['shift_by'],
                    'max_value': out.get("max_value", 1)
                })

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
    f.write("// Auto-generated DCSBIOS Data Mapping Header using structured DcsOutputEntry\n")
    f.write("#pragma once\n\n")
    f.write("#include <stdint.h>\n\n")

    f.write("struct DcsOutputEntry {\n")
    f.write("    uint16_t addr;\n")
    f.write("    uint16_t mask;\n")
    f.write("    uint8_t shift;\n")
    f.write("    uint16_t max_value;\n")
    f.write("    const char* label;\n")
    f.write("};\n\n")

    f.write("static const DcsOutputEntry DcsOutputTable[] = {\n")
    for entry in output_entries:
        f.write(f'    {{ 0x{entry["addr"]:04X}, 0x{entry["mask"]:04X}, {entry["shift"]}, {entry["max_value"]}, "{entry["label"]}" }},\n')
    f.write("};\n\n")

    f.write("static const size_t DcsOutputTableSize = sizeof(DcsOutputTable) / sizeof(DcsOutputTable[0]);\n\n")

    from collections import defaultdict

    # Group entries by address
    addr_to_indices = defaultdict(list)
    for i, entry in enumerate(output_entries):
        addr_to_indices[entry["addr"]].append(i)

    f.write("static const std::unordered_map<uint16_t, std::vector<const DcsOutputEntry*>> addressToEntries = {\n")
    for addr, indices in addr_to_indices.items():
        pointers = ', '.join(f'&DcsOutputTable[{i}]' for i in indices)
        f.write(f'    {{ 0x{addr:04X}, {{ {pointers} }} }},\n')
    f.write("};\n\n")

    f.write("struct DcsInputEntry {\n")
    f.write("    const char* label;\n")
    f.write("    uint16_t max_value;\n")
    f.write("    const char* description;\n")
    f.write("};\n\n")

    f.write("static const DcsInputEntry DcsInputTable[] = {\n")
    for entry in input_entries:
        desc = entry["description"].replace('"', '\\"')
        f.write(f'    {{ "{entry["label"]}", {entry["max_value"]}, "{desc}" }},\n')
    f.write("};\n\n")

    f.write("// ---- Enhanced Selector Info (from LUA) ----\n")
    for comment in selector_info_comments:
        f.write(comment + "\n")

print(f"[✓] Generated {output_header_file} with {len(output_entries)} output records and {len(input_entries)} input records.")

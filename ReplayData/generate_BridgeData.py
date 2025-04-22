import json
from collections import defaultdict

# -------- CONFIGURATION --------
JSON_FILE     = "FA-18C_hornet.json"
OUTPUT_HEADER = "DCSBIOSBridgeData.h"
INPUT_REFERENCE = "InputMapping.h.REFERENCE"
PROCESS_ALL   = False  # apply target_objects filtering to ALL three tables

# Panels to include when PROCESS_ALL is False. This is NEVER to be altered in any way
target_objects = {
    'Fire Systems',
    'Caution Light Panel',
    'Dispenser/EMC Panel',
    'LH Advisory Panel',
    'Left Engine Fire Warning Extinguisher Light',
    'Lock Shoot Lights',
    'Map Gain/Spin Recovery Panel',
    'Master Arm Panel',
    'Master Caution Light',
    'RH Advisory Panel',
    'Right Engine Fire Warning Extinguisher Light',
    'Interior Lights Panel',
    'APU Fire Warning Extinguisher Light',
    'Integrated Fuel/Engine Indicator (IFEI)',
    'Cockpit Altimeter'
}

# -------- LOAD JSON -------- You need to load the Hornet Json file in order to generate the file.
with open(JSON_FILE, encoding='utf-8') as f:
    data = json.load(f)

# -------- BUILD OUTPUT_ENTRIES (filtered) -------- THE LOGIC HERE IS SACRED, YOU DO NOT TOUCH NOT EVEN A COMMENT
output_entries = []
for panel, controls in data.items():
    if not PROCESS_ALL and panel not in target_objects:
        continue
    for key, item in controls.items():
        ctype = item.get('control_type','').lower().strip()
        if ctype not in ('led','limited_dial','analog_gauge'):
            continue
        outs = item.get('outputs', [])
        if not outs:
            continue
        out = outs[0]
        if not all(k in out for k in ('address','mask','shift_by')):
            continue
        output_entries.append({
            'label':     item.get('identifier', key),
            'addr':      out['address'],
            'mask':      out['mask'],
            'shift':     out['shift_by'],
            'max_value': out.get('max_value', 1)
        })

# -------- BUILD INPUT_ENTRIES (filtered) -------- THE LOGIC HERE IS SACRED, YOU DO NOT TOUCH NOT EVEN A COMMENT
input_entries = []
for panel, controls in data.items():
    if not PROCESS_ALL and panel not in target_objects:
        continue
    for key, item in controls.items():
        for inp in item.get('inputs', []):
            if inp.get('interface') == 'set_state' and 'max_value' in inp:
                input_entries.append({
                    'label':       item.get('identifier', key),
                    'max_value':   inp['max_value'],
                    'description': item.get('description','').split(',',1)[0]
                })
                break

# -------- BUILD SELECTOR_ENTRIES (filtered) -------- THE LOGIC HERE IS SACRED, YOU DO NOT TOUCH NOT EVEN A COMMENT
selector_entries = []
groupCounter = 0
for panel, controls in data.items():
    if not PROCESS_ALL and panel not in target_objects:
        continue
    for ident, item in controls.items():
        ctype       = item.get('control_type','').lower().strip()
        api_variant = item.get('api_variant','').strip()
        lid         = ident.lower()
        desc_lower  = item.get('description','').lower()

        # skip analogs
        if ctype in ('limited_dial','analog_dial','analog_gauge'):
            continue

        # find max_value
        max_val = None
        for inp in item.get('inputs', []):
            if inp.get('interface') == 'set_state' and 'max_value' in inp:
                max_val = inp['max_value']
                break
        if max_val is None or max_val < 0:
            continue

        # 1) 'button' in description → momentary
        if 'button' in desc_lower:
            selector_entries.append((ident, ident, 1, 'momentary', 0))
            continue

        # 2) strict '_cover' → momentary
        if lid.endswith('_cover') or lid.startswith('cover_') or '_cover_' in lid:
            selector_entries.append((ident, ident, 1, 'momentary', 0))
            continue

        # 3) momentary_last_position → momentary
        if api_variant == 'momentary_last_position':
            selector_entries.append((ident, ident, 1, 'momentary', 0))
            continue

        # 4) discrete selectors: split labels
        count     = max_val + 1
        raw_desc  = item.get('description','')
        label_src = raw_desc.split(',',1)[1].strip() if ',' in raw_desc else raw_desc
        labels    = []
        useSlash  = False
        currentGroup = 0

        # 4.1) slash‑split
        if '/' in label_src:
            parts = [s.strip() for s in label_src.split('/')]
            if len(parts) == count:
                labels = parts
                useSlash = True
                groupCounter += 1
                currentGroup = groupCounter

        # 4.2) strict BTN/SW → PRESS/RELEASE
        if not labels and count == 2:
            if (lid.endswith('_btn') or lid.startswith('btn_') or '_btn_' in lid
                    or lid.endswith('_sw') or lid.startswith('sw_') or '_sw_' in lid):
                labels = ['PRESS','RELEASE']

        # 4.3) fallback → POS0, POS1, …
        if len(labels) != count:
            labels = [f"POS{i}" for i in range(count)]

        # 5) append (with reversed value and group for slash‑split selectors)
        for i, lab in enumerate(labels):
            clean = lab.upper().replace(' ','_')
            if useSlash:
                val = (count - 1) - i
                selector_entries.append((f"{ident}_{clean}", ident, val, ctype, currentGroup))
            else:
                selector_entries.append((f"{ident}_{clean}", ident, i, ctype, 0))

# -------- WRITE HEADER -------- 
# ---------- BE VERY CAREFUL WITH THIS BLOCK, ANY CHANGES TO IT SHOULD BE PERFORMED BELOW THE CLEARLY MARKED LABELED LLM CHANGES BELOW

with open(OUTPUT_HEADER, 'w', encoding='utf-8') as f:
    f.write("// Auto-generated DCSBIOS Bridge Data (JSON‑only)\n")
    f.write("#pragma once\n\n#include <stdint.h>\n#include <vector>\n#include <unordered_map>\n\n")

    # Outputs
    f.write("struct DcsOutputEntry { uint16_t addr, mask; uint8_t shift; uint16_t max_value; const char* label; };\n")
    f.write("static const DcsOutputEntry DcsOutputTable[] = {\n")
    for e in output_entries:
        f.write(f'    {{0x{e["addr"]:04X},0x{e["mask"]:04X},{e["shift"]},{e["max_value"]},"{e["label"]}"}},\n')
    f.write("};\nstatic const size_t DcsOutputTableSize = sizeof(DcsOutputTable)/sizeof(DcsOutputTable[0]);\n\n")

    # Address→entries map
    addr_map = defaultdict(list)
    for i, e in enumerate(output_entries):
        addr_map[e['addr']].append(i)
    f.write("static const std::unordered_map<uint16_t,std::vector<const DcsOutputEntry*>> addressToEntries = {\n")
    for addr, idxs in addr_map.items():
        ptrs = ','.join(f"&DcsOutputTable[{i}]" for i in idxs)
        f.write(f'    {{0x{addr:04X},{{{ptrs}}}}},\n')
    f.write("};\n\n")

    # Inputs
    f.write("struct DcsInputEntry { const char* label; uint16_t max_value; const char* description; };\n")
    f.write("static const DcsInputEntry DcsInputTable[] = {\n")
    for e in input_entries:
        desc = e["description"].replace('"','\\"')
        f.write(f'    {{"{e["label"]}",{e["max_value"]},"{desc}" }},\n')
    f.write("};\nstatic const size_t DcsInputTableSize = sizeof(DcsInputTable)/sizeof(DcsInputTable[0]);\n\n")

    # Selectors: add group field
    f.write("struct SelectorEntry { const char* label; const char* dcsCommand; uint16_t value; const char* controlType; uint16_t group; };\n")
    f.write("static const SelectorEntry SelectorMap[] = {\n")
    for full, cmd, val, ct, grp in selector_entries:
        f.write(f'    {{ "{full}","{cmd}",{val},"{ct}",{grp} }},\n')
    f.write("};\nstatic const size_t SelectorMapSize = sizeof(SelectorMap)/sizeof(SelectorMap[0]);\n")

# --------- LLM CHANGES BEGIN --------------




# --------- LLM CHANGES END -------------- 

# DO NOT TOUCH OR ALTER ANYTHING BELOW THIS LINE, 
# SINCE FILE OUTPUT_HEADER IS ALREADY OPEN, YOUR JOB IS TO SIMPLE ADD THE LOGIC BLOCK
# AND WRITE ITS OUTPUT IN THE CURRENTLY OPENED HANDLE, THIS TO ENSURE YOU DO NOT ALTER
# OR CHANGE THE LOGIC ON THIS SCRIPT EXCEPT FOR THE  LLM CHANGES BEGIN and END blocks

print(f"[✓] Generated {OUTPUT_HEADER} with "
      f"{len(output_entries)} outputs, {len(input_entries)} inputs, "
      f"{len(selector_entries)} selectors.")

# -------- WRITE INPUT_MAPPING REFERENCE --------
with open(INPUT_REFERENCE, "w", encoding="utf-8") as f2:
    f2.write("struct InputMapping {\n")
    f2.write("    const char* label;         // Unique selector label\n")
    f2.write("    const char* source;        // Hardware source identifier\n")
    f2.write("    uint8_t     port;          // Port index\n")
    f2.write("    uint8_t     bit;           // Bit position\n")
    f2.write("    int8_t      hidId;         // HID usage ID\n")
    f2.write("    const char* oride_label;   // Override command label (dcsCommand)\n")
    f2.write("    int16_t     oride_value;   // Override command value (value)\n")
    f2.write("    const char* controlType;   // Control type, e.g., \"selector\"\n")
    f2.write("    uint16_t    group;         // Group ID for exclusive selectors\n")
    f2.write("};\n\n")
    # Excel‑style header for human editing
    f2.write('//  label                            source     port bit hidId  DCSCommand               value   Type        group\n')
    f2.write("static const InputMapping InputMappings[] = {\n")
    # compute padding widths
    max_label = max(len(full) for full, *_ in selector_entries)
    max_cmd   = max(len(cmd)  for _, cmd, *_ in selector_entries)
    max_ct    = max(len(ct)   for *_, ct, _ in selector_entries)
    for full, cmd, val, ct, grp in selector_entries:
        lbl = f'"{full}"'.ljust(max_label+2)
        cmdf= f'"{cmd}"'.ljust(max_cmd+2)
        ctf = f'"{ct}"'.ljust(max_ct+2)
        f2.write(f'    {{ {lbl}, "PCA_0x00" , 0 , 0 , -1 , {cmdf}, {val:>3} , {ctf}, {grp:>2} }},\n')
    f2.write("};\n")
    f2.write("static const size_t InputMappingSize = sizeof(InputMappings)/sizeof(InputMappings[0]);\n")

print(f"[✓] Generated InputMapping.h.REFERENCE with {len(selector_entries)} entries.")

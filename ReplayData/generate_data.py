#!/usr/bin/env python3
import os, re
import sys
import json
from collections import defaultdict
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# -------- CONFIGURATION --------
JSON_FILE     	= "FA-18C_hornet.json"
OUTPUT_HEADER 	= "DCSBIOSBridgeData.h"
INPUT_REFERENCE = "InputMapping.h"
LED_REFERENCE 	= "LEDMapping.h"
PROCESS_ALL   	= False  # apply target_objects filtering to ALL three tables
KNOWN_DEVICES 	= {
    "GPIO",
    "PCA9555",
    "TM1637",
    "GN1640T",
    "WS2812",
    "NONE",
}

# Panels to include when PROCESS_ALL is False. This is NEVER to be altered in any way
target_objects = {
    'Fire Systems',
    'Cockpit Altimeter',
    'LH Advisory Panel',
    'Left Engine Fire Warning Extinguisher Light',
    'Lock Shoot Lights',
    'Map Gain/Spin Recovery Panel',
    'Master Arm Panel',
    'Master Caution Light',
    'RH Advisory Panel',
    'Right Engine Fire Warning Extinguisher Light',
    'Caution Light Panel',
    'Dispenser/EMC Panel',
    'Interior Lights Panel',
    'APU Fire Warning Extinguisher Light'
}

# ——— HELPERS ———
def is_prime(n):
    if n < 2: return False
    if n % 2 == 0: return n == 2
    for i in range(3, int(n**0.5)+1, 2):
        if n % i == 0: return False
    return True

def next_prime(n):
    while not is_prime(n):
        n += 1
    return n

def djb2_hash(s, mod):
    h = 5381
    for c in s:
        h = ((h << 5) + h) + ord(c)
    return h % mod

# -------- LOAD JSON -------- You need to load the Hornet Json file in order to generate the file.
with open(JSON_FILE, encoding='utf-8') as f:
    data = json.load(f)

# -------- BUILD OUTPUT_ENTRIES (filtered) -------- THE LOGIC HERE IS SACRED, YOU DO NOT TOUCH NOT EVEN A COMMENT
control_type_map = {
    'led': 'CT_LED',
    'limited_dial': 'CT_ANALOG',
    'analog_gauge': 'CT_ANALOG',
    'selector': 'CT_SELECTOR',
    'toggle_switch': 'CT_SELECTOR',
    'action': 'CT_SELECTOR',
    'emergency_parking_brake': 'CT_SELECTOR',
    'display': 'CT_DISPLAY',
    'metadata': 'CT_METADATA'
}

output_entries = []
for panel, controls in data.items():
    if not PROCESS_ALL and panel not in target_objects:
        continue
    for key, item in controls.items():
        ctype_raw = item.get('control_type','').lower().strip()
        if ctype_raw not in control_type_map:
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
            'max_value': out.get('max_value', 1),
            'controlType': control_type_map[ctype_raw]
        })

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

# -------- BUILD TRACKED STATES (toggle + covers) --------

tracked_labels = set()

# 1. Covers: selector labels containing "_COVER"
for full, cmd, val, ct, grp in selector_entries:
    if "_COVER" in full:
        tracked_labels.add(full)

# 2. Toggles: momentary inputs where oride_label matches a CT_SELECTOR output
output_selector_labels = {e['label'] for e in output_entries if e['controlType'] == 'CT_SELECTOR'}
for full, cmd, val, ct, grp in selector_entries:
    if ct == "momentary" and cmd in output_selector_labels:
        tracked_labels.add(cmd)

# 3. Sort for stability
tracked_labels = sorted(tracked_labels)


# -------- WRITE HEADER FOR OUTPUT AND SELECTORS -------- 

with open(OUTPUT_HEADER, 'w', encoding='utf-8') as f:
    f.write("// Auto-generated DCSBIOS Bridge Data (JSON‑only) - DO NOT EDIT\n")
    f.write("#pragma once\n\n#include <stdint.h>\n#include <vector>\n#include <unordered_map>\n\n")

    # Outputs
    f.write("enum ControlType : uint8_t {\n")
    f.write("  CT_LED,\n  CT_ANALOG,\n  CT_SELECTOR,\n  CT_DISPLAY,\n  CT_METADATA\n};\n\n")

    f.write("struct DcsOutputEntry { uint16_t addr, mask; uint8_t shift; uint16_t max_value; const char* label; ControlType controlType; };\n")
    f.write("static const DcsOutputEntry DcsOutputTable[] = {\n")
    for e in output_entries:
        f.write(f'    {{0x{e["addr"]:04X},0x{e["mask"]:04X},{e["shift"]},{e["max_value"]},"{e["label"]}",{e["controlType"]}}},\n')
    f.write("};\nstatic const size_t DcsOutputTableSize = sizeof(DcsOutputTable)/sizeof(DcsOutputTable[0]);\n\n")

    # Address→entries static flat table
    addr_map = defaultdict(list)
    for i, e in enumerate(output_entries):
        addr_map[e['addr']].append(i)

    f.write("// Static flat address-to-output entry lookup\n")
    f.write("struct AddressEntry {\n")
    f.write("  uint16_t addr;\n")

    # dynamically determine max entry count per address
    max_entries_per_addr = max(len(idxs) for idxs in addr_map.values())
    f.write(f"  const DcsOutputEntry* entries[{max_entries_per_addr}]; // max entries per address\n")
    f.write("  uint8_t count;\n")
    f.write("};\n\n")

    f.write("static const AddressEntry dcsAddressTable[] = {\n")
    for addr, idxs in addr_map.items():
        ptrs = ', '.join(f"&DcsOutputTable[{i}]" for i in idxs)
        count = len(idxs)
        f.write(f"  {{ 0x{addr:04X}, {{ {ptrs} }}, {count} }},\n")
    f.write("};\n\n")

    f.write("inline const AddressEntry* findDcsOutputEntries(uint16_t addr) {\n")
    f.write("  for (uint8_t i = 0; i < sizeof(dcsAddressTable)/sizeof(dcsAddressTable[0]); ++i) {\n")
    f.write("    if (dcsAddressTable[i].addr == addr) return &dcsAddressTable[i];\n")
    f.write("  }\n")
    f.write("  return nullptr;\n")
    f.write("}\n\n")

    # Selectors: add group field
    f.write("struct SelectorEntry { const char* label; const char* dcsCommand; uint16_t value; const char* controlType; uint16_t group; };\n")
    f.write("static const SelectorEntry SelectorMap[] = {\n")
    for full, cmd, val, ct, grp in selector_entries:
        f.write(f'    {{ "{full}","{cmd}",{val},"{ct}",{grp} }},\n')
    f.write("};\nstatic const size_t SelectorMapSize = sizeof(SelectorMap)/sizeof(SelectorMap[0]);\n")

    f.write("\n// Tracked toggle & cover states\n")
    f.write("TrackedStateEntry trackedStates[] = {\n")
    for label in tracked_labels:
        f.write(f'    {{ "{label}", false }},\n')
    f.write("};\n")
    f.write("const size_t trackedStatesCount = sizeof(trackedStates)/sizeof(trackedStates[0]);\n")

print(f"[✓] Generated {OUTPUT_HEADER} with "
      f"{len(output_entries)} outputs,  "
      f"{len(selector_entries)} selectors.")

# -------- WRITE InputMapping.h from selector_entries above (label-keyed preserve) --------

# 1) load existing entries, keyed by label
existing_map = {}
line_re = re.compile(
    r'\{\s*"(?P<label>[^"]+)"\s*,\s*'        # capture the label
    r'"(?P<source>[^"]+)"\s*,\s*'
    r'(?P<port>-?\d+)\s*,\s*'
    r'(?P<bit>-?\d+)\s*,\s*'
    r'(?P<hidId>-?\d+)\s*,\s*'
    r'"(?P<cmd>[^"]+)"\s*,\s*'
    r'(?P<value>-?\d+)\s*,\s*'
    r'"(?P<type>[^"]+)"\s*,\s*'
    r'(?P<group>\d+)\s*\}\s*,'
)
if os.path.exists(INPUT_REFERENCE):
    with open(INPUT_REFERENCE, "r", encoding="utf-8") as fin:
        for line in fin:
            m = line_re.search(line)
            if not m:
                continue
            d = m.groupdict()
            existing_map[d["label"]] = {
                "label":       d["label"],
                "source":      d["source"],
                "port":        int(d["port"]),
                "bit":         int(d["bit"]),
                "hidId":       int(d["hidId"]),
                "oride_label": d["cmd"],
                "oride_value": int(d["value"]),
                "controlType": d["type"],
                "group":       int(d["group"])
            }

# 2) merge into a new list, preserving any user edits by label
merged = []
for full, cmd, val, ct, grp in selector_entries:
    if full in existing_map:
        e = existing_map[full]
        merged.append((
            e["label"], e["source"], e["port"], e["bit"],
            e["hidId"], e["oride_label"], e["oride_value"],
            e["controlType"], e["group"]
        ))
    else:
        # brand-new entry: use defaults
        merged.append((full, "PCA_0x00", 0, 0, -1, cmd, val, ct, grp))

# 3) write out merged list
with open(INPUT_REFERENCE, "w", encoding="utf-8") as f2:
    input_labels = [e[0] for e in merged]
    f2.write("// THIS FILE IS AUTO-GENERATED; ONLY EDIT INDIVIDUAL RECORDS, DO NOT ADD OR DELETE THEM HERE\n")
    f2.write("#pragma once\n\n")
    f2.write("struct InputMapping {\n")
    f2.write("    const char* label;        // Unique selector label\n")
    f2.write("    const char* source;       // Hardware source identifier\n")
    f2.write("    uint8_t     port;         // Port index\n")
    f2.write("    uint8_t     bit;          // Bit position\n")
    f2.write("    int8_t      hidId;        // HID usage ID\n")
    f2.write("    const char* oride_label;  // Override command label (dcsCommand)\n")
    f2.write("    int16_t     oride_value;  // Override command value (value)\n")
    f2.write("    const char* controlType;  // Control type, e.g., \"selector\"\n")
    f2.write("    uint16_t    group;        // Group ID for exclusive selectors\n")
    f2.write("};\n\n")
    f2.write('//  label                       source     port bit hidId  DCSCommand           value   Type        group\n')
    f2.write("static const InputMapping InputMappings[] = {\n")

    max_label = max(len(e[0]) for e in merged)
    max_cmd   = max(len(e[5]) for e in merged)
    max_type  = max(len(e[7]) for e in merged)

    for lbl, src, port, bit, hid, cmd, val, typ, gp in merged:
        lblf = f'"{lbl}"'.ljust(max_label+2)
        cmdf = f'"{cmd}"'.ljust(max_cmd+2)
        ctf  = f'"{typ}"'.ljust(max_type+2)
        f2.write(f'    {{ {lblf}, "{src}" , {port:>2} , {bit:>2} , {hid:>3} , '
                 f'{cmdf}, {val:>3} , {ctf}, {gp:>2} }},\n')

    f2.write("};\n")
    f2.write("static const size_t InputMappingSize = sizeof(InputMappings)/sizeof(InputMappings[0]);\n\n")

    # 4) Generate static hash table for InputMappings[]
    desired = len(input_labels) * 2
    INPUT_TABLE_SIZE = next_prime(max(desired, 53))

    input_hash_table = ["{nullptr, nullptr}"] * INPUT_TABLE_SIZE

    for idx, label in enumerate(input_labels):
        h = djb2_hash(label, INPUT_TABLE_SIZE)
        for probe in range(INPUT_TABLE_SIZE):
            if input_hash_table[h] == "{nullptr, nullptr}":
                input_hash_table[h] = f'{{"{label}", &InputMappings[{idx}]}}'
                break
            h = (h + 1) % INPUT_TABLE_SIZE
        else:
            print(f"❌ Input hash table full! TABLE_SIZE={INPUT_TABLE_SIZE} too small", file=sys.stderr)
            sys.exit(1)

    f2.write(f"\n// Static hash lookup table for InputMappings[]\n")
    f2.write("struct InputHashEntry { const char* label; const InputMapping* mapping; };\n")
    f2.write(f"static const InputHashEntry inputHashTable[{INPUT_TABLE_SIZE}] = {{\n")
    for entry in input_hash_table:
        f2.write(f"  {entry},\n")
    f2.write("};\n\n")

    f2.write("// DJB2 hash function for input labels\n")
    f2.write("constexpr uint16_t inputHash(const char* str) {\n")
    f2.write("  uint16_t hash = 5381;\n")
    f2.write("  while (*str) { hash = ((hash << 5) + hash) + *str++; }\n")
    f2.write("  return hash;\n")
    f2.write("}\n\n")

    f2.write("inline const InputMapping* findInputByLabel(const char* label) {\n")
    f2.write(f"  uint16_t startH = inputHash(label) % {INPUT_TABLE_SIZE};\n")
    f2.write(f"  for (uint16_t i = 0; i < {INPUT_TABLE_SIZE}; ++i) {{\n")
    f2.write(f"    uint16_t idx = (startH + i >= {INPUT_TABLE_SIZE}) ? (startH + i - {INPUT_TABLE_SIZE}) : (startH + i);\n")
    f2.write("    const auto& entry = inputHashTable[idx];\n")
    f2.write("    if (!entry.label) continue;\n")
    f2.write("    if (strcmp(entry.label, label) == 0) return entry.mapping;\n")
    f2.write("  }\n")
    f2.write("  return nullptr;\n")
    f2.write("}\n")

print(f"[✓] Generated {INPUT_REFERENCE} with {len(merged)} entries.")

# --------------------------- Generate our LEDMappings.h file -----------------------------------------------

def generate_comment(device, info_type, info_values):
    info = [x.strip() for x in info_values.split(",")]
    if device == "GPIO" and len(info) >= 1:
        return f"// GPIO {info[0]}"
    if device == "PCA9555" and len(info) >= 3:
        return f"// PCA {info[0]} Port {info[1]} Bit {info[2]}"
    if device == "TM1637" and len(info) >= 4:
        return f"// TM1637 CLK {info[0]} DIO {info[1]} Seg {info[2]} Bit {info[3]}"
    if device == "GN1640T" and len(info) >= 3:
        return f"// GN1640 Addr {info[0]} Col {info[1]} Row {info[2]}"
    if device == "WS2812" and len(info) >= 1:
        return f"// WS2812 Index {info[0]}"
    return "// No Info"

# ——— 1) PARSE existing LEDControl_Lookup.h (if any) ———
existing = {}
if os.path.exists(LED_REFERENCE):
    with open(LED_REFERENCE, "r", encoding="utf-8") as f:
        txt = f.read()

    entry_re = re.compile(
        r'\{\s*"(?P<label>[^"]+)"\s*,\s*DEVICE_(?P<device>[A-Z0-9_]+)\s*,'
        r'\s*\{\s*\.(?P<info_type>[a-zA-Z0-9_]+)\s*=\s*\{\s*(?P<info_values>[^}]+)\}\s*\}'
        r'\s*,\s*(?P<dimmable>true|false)\s*,\s*(?P<activeLow>true|false)\s*\}',
        re.MULTILINE
    )

    for m in entry_re.finditer(txt):
        d = m.groupdict()
        existing[d["label"]] = {
            "device":      d["device"],
            "info_type":   d["info_type"],
            "info_values": d["info_values"].strip(),
            "dimmable":    d["dimmable"],
            "activeLow":   d["activeLow"],
        }


# ——— 2) PARSE DcsOutputTable for all labels (only CT_LED, CT_ANALOG) ———
if not os.path.exists(OUTPUT_HEADER):
    print(f"❌ Cannot find `{OUTPUT_HEADER}` – adjust OUTPUT_HEADER path.", file=sys.stderr)
    sys.exit(1)

with open(OUTPUT_HEADER, "r", encoding="utf-8") as f:
    dcs = f.read()

dcs_re = re.compile(
    r'\{\s*0x[0-9A-Fa-f]+\s*,\s*0x[0-9A-Fa-f]+\s*,\s*\d+\s*,\s*\d+\s*,\s*"([^"]+)"\s*,\s*(CT_\w+)\s*\}'
)

labels = []
for m in dcs_re.finditer(dcs):
    label = m.group(1)
    control_type = m.group(2)
    if control_type in ("CT_LED", "CT_ANALOG"):
        labels.append(label)

# Duplicate label detection
if len(labels) != len(set(labels)):
    print("⚠️ WARNING: Duplicate labels detected in DCS table!", file=sys.stderr)

print(f"✅ Found {len(labels)} LED/Analog labels in DcsOutputTable ({OUTPUT_HEADER})")

# ——— 3) Compute table size dynamically (load ≤ 50%) ———
desired = len(labels) * 2
TABLE_SIZE = next_prime(max(desired, 53))
print(f"🔢 Using TABLE_SIZE = {TABLE_SIZE} (next prime ≥ {desired})")

# ——— 4) Build the new panelLEDs[] lines ———
# for clean alignment, find max label width
max_label_len = max(len(label) for label in labels) if labels else 0

final_panel = []
for label in labels:
    padded_label = f'"{label}"'.ljust(max_label_len+2)
    if label in existing:
        e = existing[label]
        dev = e["device"]
        if dev not in KNOWN_DEVICES:
            print(f"⚠️  WARNING: DEVICE_{dev} on `{label}` not in KNOWN_DEVICES")
        line = (
            f'  {{ {padded_label}, DEVICE_{dev.ljust(8)}, '
            f'{{.{e["info_type"]} = {{{e["info_values"]}}}}}, '
            f'{e["dimmable"]}, {e["activeLow"]} }}, {generate_comment(dev, e["info_type"], e["info_values"])}'
        )
    else:
        line = (
            f'  {{ {padded_label}, DEVICE_NONE    , '
            f'{{.gpioInfo = {{0}}}}, false, false }}'
        )
    final_panel.append(line)

# ——— 5) Build & probe the hash table ———
hash_table = ["{nullptr, nullptr}"] * TABLE_SIZE
for idx, label in enumerate(labels):
    h = djb2_hash(label, TABLE_SIZE)
    for probe in range(TABLE_SIZE):
        if hash_table[h] == "{nullptr, nullptr}":
            hash_table[h] = f'{{"{label}", &panelLEDs[{idx}]}}'
            break
        h = (h + 1) % TABLE_SIZE
    else:
        print(f"❌ Hash table full! TABLE_SIZE={TABLE_SIZE} too small", file=sys.stderr)
        sys.exit(1)

# ——— 6) Emit the new LEDMapping.h ———
with open(LED_REFERENCE, "w", encoding="utf-8") as out:
    out.write("// THIS FILE IS AUTO-GENERATED; ONLY EDIT INDIVIDUAL LED/GAUGE RECORDS, DO NOT ADD OR DELETE THEM HERE\n")
    out.write("#pragma once\n\n")

    out.write("// Embedded LEDMapping structure and enums\n")
    out.write("enum LEDDeviceType {\n")
    for dev in KNOWN_DEVICES:
        out.write(f"  DEVICE_{dev},\n")
    out.write("};\n\n")

    out.write("struct LEDMapping {\n")
    out.write("  const char* label;\n")
    out.write("  LEDDeviceType deviceType;\n")
    out.write("  union {\n")
    out.write("    struct { uint8_t gpio; } gpioInfo;\n")
    out.write("    struct { uint8_t address; uint8_t port; uint8_t bit; } pcaInfo;\n")
    out.write("    struct { uint8_t clkPin; uint8_t dioPin; uint8_t segment; uint8_t bit; } tm1637Info;\n")
    out.write("    struct { uint8_t address; uint8_t column; uint8_t row; } gn1640Info;\n")
    out.write("    struct { uint8_t index; } ws2812Info;\n")
    out.write("  } info;\n")
    out.write("  bool dimmable;\n")
    out.write("  bool activeLow;\n")
    out.write("};\n\n")

    out.write("// Auto-generated panelLEDs array\n")
    out.write("static const LEDMapping panelLEDs[] = {\n")
    out.write(",\n".join(final_panel))
    out.write("\n};\n\n")

    out.write("static constexpr uint16_t panelLEDsCount = sizeof(panelLEDs)/sizeof(panelLEDs[0]);\n\n")

    out.write("// Auto-generated hash table\n")
    out.write("struct LEDHashEntry { const char* label; const LEDMapping* led; };\n")
    out.write(f"static const LEDHashEntry ledHashTable[{TABLE_SIZE}] = {{\n")
    for entry in hash_table:
        out.write(f"  {entry},\n")
    out.write("};\n\n")

    out.write("// djb2 hash function\n")
    out.write("constexpr uint16_t ledHash(const char* str) {\n")
    out.write("  uint16_t hash = 5381;\n")
    out.write("  while (*str) { hash = ((hash << 5) + hash) + *str++; }\n")
    out.write("  return hash;\n")  # No % TABLE_SIZE here
    out.write("}\n\n")

    out.write("// findLED lookup\n")
    out.write("inline const LEDMapping* findLED(const char* label) {\n")
    out.write(f"  uint16_t startH = ledHash(label) % {TABLE_SIZE};\n")
    out.write(f"  for (uint16_t i = 0; i < {TABLE_SIZE}; ++i) {{\n")
    out.write(f"    uint16_t idx = (startH + i >= {TABLE_SIZE}) ? (startH + i - {TABLE_SIZE}) : (startH + i);\n")
    out.write("    const auto& entry = ledHashTable[idx];\n")
    out.write("    if (!entry.label) continue;\n")
    out.write("    if (strcmp(entry.label, label) == 0) return entry.led;\n")
    out.write("  }\n")
    out.write("  return nullptr;\n")
    out.write("}\n")

print(f"✅ {LED_REFERENCE} fully regenerated with dynamic enum from KNOWN_DEVICES.")
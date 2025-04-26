#!/usr/bin/env python3
import re
import os
import sys

# ─── CONFIG ────────────────────────────────────────────────────────────────────

# where to find the DCSBIOS bridge data
DCS_FILE      = os.path.join("src", "LABELS", "DCSBIOSBridgeData.h")
# where to read/write your LED lookup
LOOKUP_FILE   = os.path.join("src", "LABELS", "LEDControl_Lookup.h")
# size of the hash table (must be prime and >= number of LEDs * 2)
TABLE_SIZE    = 53

# ─── HELPERS ───────────────────────────────────────────────────────────────────

def die(msg):
    print("❌", msg, file=sys.stderr)
    sys.exit(1)

def led_hash(label):
    """djb2 variant, mod TABLE_SIZE"""
    h = 5381
    for c in label:
        h = ((h << 5) + h) + ord(c)
    return h % TABLE_SIZE

# ─── 1) LOAD EXISTING lookups ─────────────────────────────────────────────────

existing = {}
entry_re = re.compile(
    r'\{\s*"([^"]+)"\s*,\s*(?:DEVICE_)?([A-Za-z0-9_]+)\s*,\s*'          # "label", DEVICE_name or bare
    r'\{\s*\.\s*([A-Za-z0-9_]+)\s*=\s*\{([^}]*)\}\s*\}\s*,\s*'         # { .infoType = {values} },
    r'(true|false)'                                                   # dimmable
    r'(?:\s*,\s*(true|false))?'                                       # optional activeLow
    r'\s*\},?',                                                       # trailing },
    re.MULTILINE
)

if os.path.exists(LOOKUP_FILE):
    text = open(LOOKUP_FILE, encoding="utf-8").read()
    for m in entry_re.finditer(text):
        label       = m.group(1)
        device      = m.group(2)
        info_type   = m.group(3)
        info_values = m.group(4).strip()
        dimmable    = m.group(5)
        active_low  = m.group(6) or "false"
        existing[label] = {
            "device":      device,
            "info_type":   info_type,
            "info_values": info_values,
            "dimmable":    dimmable,
            "activeLow":   active_low
        }
else:
    print("⚠️  No existing LEDControl_Lookup.h found; starting from scratch")

# ─── 2) SCAN DCSBIOSBridgeData for labels ────────────────────────────────────

if not os.path.exists(DCS_FILE):
    die(f"{DCS_FILE} not found; please run from repo root")

dcs_text = open(DCS_FILE, encoding="utf-8").read()
dcs_label_re = re.compile(r'\{\s*0x[0-9A-Fa-f]+,\s*0x[0-9A-Fa-f]+,\s*\d+,\s*\d+,\s*"([^"]+)"\s*\}')
dcs_labels = [m.group(1) for m in dcs_label_re.finditer(dcs_text)]
print(f"✅ Found {len(dcs_labels)} labels in DcsOutputTable ({DCS_FILE})")

# ─── 3) BUILD new panelLEDs[] ────────────────────────────────────────────────

panel_leds = []
hash_table = ["{nullptr, nullptr}"] * TABLE_SIZE

for idx, label in enumerate(dcs_labels):
    if label in existing:
        e = existing[label]
        line = (
            f'  {{"{label}", DEVICE_{e["device"]}, '
            f'{{.{e["info_type"]} = {{{e["info_values"]}}}}}, '
            f'{e["dimmable"]}'
        )
        if e["activeLow"] != "false":
            line += f', {e["activeLow"]}'
        line += "}"
    else:
        # brand-new label → placeholder
        line = f'  {{"{label}", DEVICE_NONE, {{.gpioInfo = {{0}}}}, false}}'
    panel_leds.append(line)

    # insert into open-addressing hash_table
    h = led_hash(label)
    for probe in range(TABLE_SIZE):
        pos = (h + probe) % TABLE_SIZE
        if hash_table[pos] == "{nullptr, nullptr}":
            hash_table[pos] = f'{{"{label}", &panelLEDs[{idx}]}}'
            break
    else:
        die(f"Hash table full! TABLE_SIZE={TABLE_SIZE} too small")

# ─── 4) WRITE OUT LEDControl_Lookup.h ────────────────────────────────────────

with open(LOOKUP_FILE, "w", encoding="utf-8") as out:
    out.write("// THIS FILE IS AUTO-GENERATED; do not edit by hand\n")
    out.write("#pragma once\n\n")
    out.write("#include \"src/LEDControl.h\"\n\n")

    out.write("// Auto-generated panelLEDs array\n")
    out.write("static const LEDMapping panelLEDs[] = {\n")
    out.write(",\n".join(panel_leds))
    out.write("\n};\n\n")

    out.write("static constexpr uint16_t panelLEDsCount = sizeof(panelLEDs)/sizeof(panelLEDs[0]);\n\n")

    out.write("// Auto-generated hash table\n")
    out.write("struct LEDHashEntry { const char* label; const LEDMapping* led; };\n")
    out.write(f"static const LEDHashEntry ledHashTable[{TABLE_SIZE}] = {{\n")
    out.write(",\n".join("  "+e for e in hash_table))
    out.write("\n};\n\n")

    out.write("// djb2-based constexpr hash\n")
    out.write("constexpr uint16_t ledHash(const char* str) {\n")
    out.write("    uint16_t h = 5381;\n")
    out.write("    while (*str) { h = ((h << 5) + h) + *str++; }\n")
    out.write(f"    return h % {TABLE_SIZE};\n")
    out.write("}\n\n")

    out.write("// findLED via open addressing\n")
    out.write("inline const LEDMapping* findLED(const char* label) {\n")
    out.write("    uint16_t h = ledHash(label);\n")
    out.write(f"    for (int i = 0; i < {TABLE_SIZE}; ++i) {{\n")
    out.write("        const auto& e = ledHashTable[(h + i) % ")
    out.write(f"{TABLE_SIZE}];\n")
    out.write("        if (!e.label) return nullptr;\n")
    out.write("        if (strcmp(e.label, label) == 0) return e.led;\n")
    out.write("    }\n")
    out.write("    return nullptr;\n")
    out.write("}\n")

print(f"✅ {LOOKUP_FILE} regenerated successfully")

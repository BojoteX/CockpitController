import json

# Files
JSON_HORNET = "FA-18C_hornet.json"
JSON_STREAM = "dcsbios_data.json"

def dcs_hash(addr, mask, shift):
    return ((addr << 16) ^ mask ^ (shift << 1)) & 0xFFFFFFFF

# Parse JSON for valid output entries
output_map = {}
meta_map = {}

with open(JSON_HORNET, "r", encoding="utf-8") as f:
    data = json.load(f)

for panel, controls in data.items():
    for ident, item in controls.items():
        for out in item.get("outputs", []):
            if all(k in out for k in ("address", "mask", "shift_by")):
                addr = out["address"]
                mask = out["mask"]
                shift = out["shift_by"]
                h = dcs_hash(addr, mask, shift)
                output_map.setdefault(h, []).append(ident)
                meta_map[h] = (addr, mask, shift, out.get("max_value", 1))

# Decode binary stream from DCS-BIOS
def decode_stream(blob):
    state = 'WAIT_SYNC'
    sync = 0
    addr = 0
    count = 0
    low = 0
    idx = 0
    result = []

    while idx < len(blob):
        b = blob[idx]
        idx += 1

        if state == 'WAIT_SYNC':
            sync = sync + 1 if b == 0x55 else 0
            if sync == 4:
                state = 'ADDRESS_LOW'
                sync = 0
            continue

        elif state == 'ADDRESS_LOW':
            addr = b
            state = 'ADDRESS_HIGH'

        elif state == 'ADDRESS_HIGH':
            addr |= b << 8
            if addr == 0x5555:
                state = 'WAIT_SYNC'
                sync = 2
            else:
                state = 'COUNT_LOW'

        elif state == 'COUNT_LOW':
            count = b
            state = 'COUNT_HIGH'

        elif state == 'COUNT_HIGH':
            count |= b << 8
            state = 'DATA_LOW' if count > 0 else 'ADDRESS_LOW'

        elif state == 'DATA_LOW':
            low = b
            state = 'DATA_HIGH'

        elif state == 'DATA_HIGH':
            value = low | (b << 8)
            result.append((addr, value))
            addr += 2
            count -= 2
            state = 'DATA_LOW' if count > 0 else 'ADDRESS_LOW'

    return result

# Main timeline log
print("==== DCS-BIOS MATCHED EVENT LOG ====\n")

t = 0.0
prev = {}

with open(JSON_STREAM, "r") as f:
    stream = json.load(f)

for packet in stream:
    t += float(packet.get("timing", 0))
    blob = bytes.fromhex(packet["data"])
    decoded = decode_stream(blob)

    for addr, value in decoded:
        for h, (m_addr, m_mask, m_shift, max_val) in meta_map.items():
            if m_addr != addr:
                continue
            extracted = (value & m_mask) >> m_shift
            if prev.get(h) == extracted:
                continue
            prev[h] = extracted
            for label in output_map[h]:
                print(f"[{t:8.3f}] [MATCH] Label = {label} → value = {extracted}")

print("\n==== END OF MATCH ====")

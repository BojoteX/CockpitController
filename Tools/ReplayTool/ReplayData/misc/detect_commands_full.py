
import json

# Load LED and STRING output definitions from FA-18C_hornet.json
def load_outputs(json_path):
    with open(json_path, "r") as f:
        data = json.load(f)
    leds = {}
    strings = {}
    for panel, controls in data.items():
        for ident, item in controls.items():
            outputs = item.get("outputs", [])
            for out in outputs:
                addr = out.get("address")
                mask = out.get("mask")
                shift = out.get("shift_by")
                if out.get("type") == "integer" and mask is not None:
                    if addr not in leds:
                        leds[addr] = {}
                    leds[addr][ident] = (mask, shift)
                elif out.get("type") == "string":
                    strings[addr] = {
                        "label": ident,
                        "length": out.get("max_length", 8)  # default to 8 if not specified
                    }
    return leds, strings

# Load stream from replay log JSON file
def load_stream(file_path):
    with open(file_path, "r") as f:
        data = json.load(f)
    stream_bytes = bytearray()
    for item in data:
        hex_str = item.get("data", "")
        if hex_str:
            stream_bytes.extend(bytes.fromhex(hex_str))
    return stream_bytes

if __name__ == "__main__":
    LED_DEFS, STRING_DEFS = load_outputs("FA-18C_hornet.json")
    stream = load_stream("dcsbios_data.json")

    led_state = {name: 0 for addr in LED_DEFS for name in LED_DEFS[addr]}
    string_state = {info["label"]: "" for info in STRING_DEFS.values()}
    buffer = {}

    sync_bytes_seen = 0
    current_address = 0
    remaining_count = 0
    current_word_low = 0
    state = "WAIT_SYNC"

    for byte in stream:
        if byte == 0x55:
            sync_bytes_seen += 1
        else:
            sync_bytes_seen = 0

        if sync_bytes_seen == 4:
            sync_bytes_seen = 0
            state = "ADDRESS_LOW"
            continue

        if state == "WAIT_SYNC":
            continue

        elif state == "ADDRESS_LOW":
            current_address = byte
            state = "ADDRESS_HIGH"

        elif state == "ADDRESS_HIGH":
            current_address |= (byte << 8)
            if current_address == 0x5555:
                state = "WAIT_SYNC"
                sync_bytes_seen = 2
            else:
                state = "COUNT_LOW"

        elif state == "COUNT_LOW":
            remaining_count = byte
            state = "COUNT_HIGH"

        elif state == "COUNT_HIGH":
            remaining_count |= (byte << 8)
            if remaining_count == 0:
                state = "WAIT_SYNC"
            else:
                state = "DATA_LOW"

        elif state == "DATA_LOW":
            current_word_low = byte
            state = "DATA_HIGH"

        elif state == "DATA_HIGH":
            word_value = current_word_low | (byte << 8)

            # LED updates
            if current_address in LED_DEFS:
                for led_name, (mask, shift) in LED_DEFS[current_address].items():
                    value = (word_value & mask) >> shift
                    if value != led_state[led_name]:
                        led_state[led_name] = value
                        state_str = "ON " if value else "OFF"
                        print(f"  • {led_name:<30} →  {state_str}")

            # STRING buffers
            elif current_address in STRING_DEFS:
                label = STRING_DEFS[current_address]["label"]
                length = STRING_DEFS[current_address]["length"]
                buffer[label] = bytearray()
                buffer[label].append(current_word_low)
                buffer[label].append(byte)

                while remaining_count > 2:
                    current_address += 2
                    remaining_count -= 2
                    state = "DATA_LOW"
                    break
                else:
                    text = buffer[label].decode("ascii", errors="replace").strip()
                    if label == "_ACFT_NAME" and text:
                        print(f"\n🚀 Mission Start Detected! Aircraft = {text}\n")
                    elif label in string_state and string_state[label] != text:
                        string_state[label] = text
                        print(f"  → {label:<20} = \"{text}\"")
                    state = "ADDRESS_LOW"
                    continue

            current_address += 2
            remaining_count -= 2
            state = "DATA_LOW" if remaining_count > 0 else "ADDRESS_LOW"

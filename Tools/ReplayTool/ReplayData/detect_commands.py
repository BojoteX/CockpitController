import json
import time

# Load LED_DEFINITIONS dynamically from FA-18C_hornet.json
def load_led_definitions(json_path):
    with open(json_path, "r") as f:
        data = json.load(f)
    led_defs = {}
    for panel, controls in data.items():
        for ident, item in controls.items():
            if item.get("control_type", "").lower() == "led":
                for out in item.get("outputs", []):
                    addr = out.get("address")
                    mask = out.get("mask")
                    shift = out.get("shift_by")
                    if addr is not None and mask is not None and shift is not None:
                        if addr not in led_defs:
                            led_defs[addr] = {}
                        led_defs[addr][ident] = (mask, shift)
    return led_defs

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
    LED_DEFINITIONS = load_led_definitions("FA-18C_hornet.json")
    stream = load_stream("dcsbios_data.json")

    frame_count = 0
    frame_start_time = time.time()
    frames_since_last_event = 0
    led_state = {name: 0 for addr in LED_DEFINITIONS for name in LED_DEFINITIONS[addr]}

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
            frame_count += 1
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

            any_event_triggered = False
            if current_address in LED_DEFINITIONS:
                for led_name, (mask, shift) in LED_DEFINITIONS[current_address].items():
                    bit_value = (word_value & mask) >> shift
                    if bit_value != led_state[led_name]:
                        if frames_since_last_event > 0:
                            elapsed = time.time() - frame_start_time
                            print(f"(No output for {frames_since_last_event} frame(s), {elapsed:.2f}s elapsed)")
                            frame_start_time = time.time()
                            frames_since_last_event = 0
                        state_str = "ON" if bit_value else "OFF"
                        print(f"Frame {frame_count}: [0x{current_address:04X}] {led_name} -> turned {state_str} (value={bit_value})")
                        led_state[led_name] = bit_value
                        any_event_triggered = True

            if not any_event_triggered:
                frames_since_last_event += 1

            current_address += 2
            remaining_count -= 2
            state = "DATA_LOW" if remaining_count > 0 else "ADDRESS_LOW"
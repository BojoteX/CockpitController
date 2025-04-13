import json

INPUT_JSON = 'dcsbios_data.json'
OUTPUT_HEADER = 'DcsbiosReplayData.h'

parsed_frames = []

with open(INPUT_JSON, 'r') as f:
    replay_data = json.load(f)

for item in replay_data:
    try:
        delay = float(item.get('timing', 0))
    except:
        delay = 0.0

    hex_data = item.get('data', '')
    blob = bytes.fromhex(hex_data)

    state = 'WAIT_SYNC'
    sync_bytes_seen = 0
    current_address = 0
    remaining_count = 0
    current_word_low = 0

    idx = 0
    while idx < len(blob):
        byte = blob[idx]
        idx += 1

        if state == 'WAIT_SYNC':
            if byte == 0x55:
                sync_bytes_seen += 1
                if sync_bytes_seen == 4:
                    state = 'ADDRESS_LOW'
                    sync_bytes_seen = 0
            else:
                sync_bytes_seen = 0
            continue

        elif state == 'ADDRESS_LOW':
            current_address = byte
            state = 'ADDRESS_HIGH'

        elif state == 'ADDRESS_HIGH':
            current_address |= (byte << 8)
            if current_address == 0x5555:
                state = 'WAIT_SYNC'
                sync_bytes_seen = 2
            else:
                state = 'COUNT_LOW'

        elif state == 'COUNT_LOW':
            remaining_count = byte
            state = 'COUNT_HIGH'

        elif state == 'COUNT_HIGH':
            remaining_count |= (byte << 8)
            if remaining_count == 0:
                state = 'ADDRESS_LOW'
            else:
                state = 'DATA_LOW'

        elif state == 'DATA_LOW':
            current_word_low = byte
            state = 'DATA_HIGH'

        elif state == 'DATA_HIGH':
            word_value = current_word_low | (byte << 8)
            parsed_frames.append((delay, current_address, word_value))
            current_address += 2
            remaining_count -= 2
            state = 'DATA_LOW' if remaining_count > 0 else 'ADDRESS_LOW'

with open(OUTPUT_HEADER, 'w') as f:
    f.write('// Auto-generated from dcsbios_data.json via replay-log.py logic\n')
    f.write('#pragma once\n\n')
    f.write('#include <Arduino.h>\n\n')
    f.write('void runDcsbiosReplayFromJson();\n')
    f.write('void runDcsbiosProtocolReplay();\n\n')
    f.write('struct DcsbiosFrame {\n')
    f.write('    float delay;\n')
    f.write('    uint16_t address;\n')
    f.write('    uint16_t data;\n')
    f.write('};\n\n')
    f.write('const DcsbiosFrame replayFrames[] = {\n')

    for delay, addr, data in parsed_frames:
        f.write(f'    {{ {delay:.4f}f, 0x{addr:04X}, 0x{data:04X} }},\n')

    f.write('};\n\n')
    f.write('const size_t replayFrameCount = sizeof(replayFrames) / sizeof(replayFrames[0]);\n')
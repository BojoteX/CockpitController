import json
import re

# === CONFIGURATION ===
HEADER_FILE = "DCSBIOSBridgeData.h"
REPLAY_FILE = "DcsbiosReplayData.h"

# === Parse DCSBIOSBridgeData.h for output entries ===
entry_pattern = re.compile(
    r'\{\s*0x([0-9A-Fa-f]{4}),\s*0x([0-9A-Fa-f]{4}),\s*(\d+),\s*(\d+),\s*"([^"]+)"\s*\}'
)
entries = []
with open(HEADER_FILE, "r") as f:
    for line in f:
        m = entry_pattern.search(line)
        if m:
            entries.append({
                "address": int(m.group(1), 16),
                "mask": int(m.group(2), 16),
                "shift": int(m.group(3)),
                "max_value": int(m.group(4)),
                "label": m.group(5)
            })

addr_to_entries = {}
for entry in entries:
    addr_to_entries.setdefault(entry["address"], []).append(entry)

# === Parse replayFrames[] from .h ===
frame_pattern = re.compile(r'\{\s*([0-9.]+)f,\s*0x([0-9A-Fa-f]{4}),\s*0x([0-9A-Fa-f]{4})\s*\}')
frames = []
with open(REPLAY_FILE, "r") as f:
    for line in f:
        m = frame_pattern.search(line)
        if m:
            frames.append({
                "delay": float(m.group(1)),
                "address": int(m.group(2), 16),
                "data": int(m.group(3), 16)
            })

# === Replay logic (ESP32 behavior) ===
timeline = []
prev = {}
current_time = 0.0

for frame in frames:
    current_time += frame["delay"]
    addr = frame["address"]
    data = frame["data"]

    if addr not in addr_to_entries:
        continue

    for entry in addr_to_entries[addr]:
        val = (data & entry["mask"]) >> entry["shift"]
        label = entry["label"]

        if prev.get(label, 0) != val:
            prev[label] = val
            if entry["max_value"] <= 1:
                timeline.append(f"[{current_time:8.3f}s] [LED] {label} → {'ON' if val else 'OFF'}")
            else:
                intensity = (val * 100) // entry["max_value"]
                if intensity < 10:
                    timeline.append(f"[{current_time:8.3f}s] [LED] {label} → OFF")
                else:
                    if intensity > 90:
                        intensity = 100
                    timeline.append(f"[{current_time:8.3f}s] [LED] {label} → ON ({intensity}%)")

# === Output ===
print("\nDCSBIOS Replay Timeline:")
print("-" * 60)
for line in timeline:
    print(line)
print("-" * 60)
print(f"[✓] Total events logged: {len(timeline)}")
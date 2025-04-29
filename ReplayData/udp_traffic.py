import socket
import json
import time
import binascii
import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# === CONFIGURATION ===
UDP_TARGET_IP = "127.0.0.1"    # localhost (socat listens here)
UDP_TARGET_PORT = 5010         # DCS-BIOS standard port
INPUT_JSON_FILE = "dcsbios_data.json"

# === SETUP UDP SOCKET ===
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# === LOAD JSON DATA ONCE ===
with open(INPUT_JSON_FILE, "r") as f:
    frames = json.load(f)

print(f"Loaded {len(frames)} frames from {INPUT_JSON_FILE}")
print(f"Sending to {UDP_TARGET_IP}:{UDP_TARGET_PORT}")
print("🔁 Looping forever... Press Ctrl+C to stop.")

# === MAIN LOOP (infinite replay) ===
while True:
    first = True
    start_time = time.time()

    for frame in frames:
        delay = frame.get("timing", 0)
        hex_data = frame.get("data", "")

        if not hex_data:
            continue

        # Wait for frame timing (skip wait for first frame)
        if not first:
            time.sleep(delay)
        first = False

        try:
            binary_data = binascii.unhexlify(hex_data)
            sock.sendto(binary_data, (UDP_TARGET_IP, UDP_TARGET_PORT))
        except Exception as e:
            print(f"⚠️ Error sending frame: {e}")

    elapsed_total = time.time() - start_time
    print(f"✅ Completed one full cycle in {elapsed_total:.2f} seconds. Restarting...")

sock.close()

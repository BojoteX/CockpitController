import serial
import time
import threading
import json

# Track the start time of the flood
flood_start = None

# Optional: set to None to log all, or list keywords to highlight
WATCH_KEYWORDS = ["HMD_OFF_BRT", "MASTER_ARM"]

def read_messages(file_path):
    with open(file_path, "r") as f:
        data = json.load(f)
    return [(item['timing'], bytes.fromhex(item['data'])) for item in data]

def read_from_device(ser):
    global flood_start
    while True:
        if ser.in_waiting > 0:
            raw = ser.read(ser.in_waiting).decode(errors='replace').strip()
            if raw:
                timestamp = time.time() - flood_start if flood_start else 0
                for line in raw.splitlines():
                    if WATCH_KEYWORDS is None or any(kw in line for kw in WATCH_KEYWORDS):
                        print(f"\n[ESP32 @ {timestamp:.4f}s]: {line}")
                    else:
                        print(line, end='')

if __name__ == "__main__":
    com_port_number = input("Enter a COM Port Number: ").strip()
    com_port = f'COM{com_port_number}'

    messages = read_messages("dcsbios_data.json")

    with serial.Serial(com_port, 250000, timeout=0) as ser:
        threading.Thread(target=read_from_device, args=(ser,), daemon=True).start()

        print(f"[+] Flooding {com_port} with DCS-BIOS stream...")

        first_loop = True
        while True:
            loop_messages = messages if first_loop else messages[1:]
            flood_start = time.time()

            for elapsed_time, data in loop_messages:
                try:
                    elapsed_time_float = float(elapsed_time)
                except ValueError:
                    continue

                ser.write(data)
                time.sleep(elapsed_time_float)

            first_loop = False

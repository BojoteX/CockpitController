import socket

# === Configuration ===
UDP_PORT = 12345
BUFFER_SIZE = 1024  # Adjust if you're sending larger packets

# === Create UDP socket ===
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', UDP_PORT))

print(f"[✓] Listening for ESP32 debug messages on UDP port {UDP_PORT}...\n")

try:
    while True:
        data, addr = sock.recvfrom(BUFFER_SIZE)
        msg = data.decode('utf-8', errors='replace').strip()
        print(f"[{addr[0]}] {msg}")
        # Optional: log to file
        # with open("debug_log.txt", "a") as f:
        #     f.write(f"{msg}\n")
except KeyboardInterrupt:
    print("\n[✋] Stopped by user.")
finally:
    sock.close()

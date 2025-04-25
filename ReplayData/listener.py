import socket

# Show IP of this machine
hostname = socket.gethostname()
local_ip = socket.gethostbyname(hostname)

# === Configuration ===
UDP_PORT = 4210
BUFFER_SIZE = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('', UDP_PORT))

print(f"[✓] Listening for ESP32 debug messages on IP address: {local_ip} UDP port {UDP_PORT}...\n")

try:
    while True:
        data, addr = sock.recvfrom(BUFFER_SIZE)
        msg = data.decode('utf-8', errors='replace').strip()
        print(f"[{addr[0]}] {msg}")
except KeyboardInterrupt:
    print("\n[✋] Stopped by user.")
finally:
    sock.close()

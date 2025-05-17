import socket
import os
import argparse
from datetime import datetime

# Change to script directory
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# Discover our local IP address
def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]
    except:
        return "127.0.0.1"

# Argument parsing
parser = argparse.ArgumentParser(description="UDP Logger with optional IP filter")
parser.add_argument("--ip", help="Only log messages from this IP address", default=None)
args = parser.parse_args()

LOCAL_IP = get_local_ip()
PORT = 4210

# Setup socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', PORT))

print(f"🔭 Listening for UDP messages on {LOCAL_IP} port {PORT}...")
if args.ip:
    print(f"🎯 Filtering to only show messages from IP: {args.ip}")
print()

while True:
    try:
        data, addr = sock.recvfrom(1024)
        sender_ip, sender_port = addr
        if args.ip and sender_ip != args.ip:
            continue  # Skip unmatched IPs

        msg = data.decode('utf-8', errors='ignore').strip()
        timestamp = datetime.now().strftime('%H:%M:%S')
        print(f"[{timestamp} - {sender_ip}] {msg}")

    except Exception as e:
        print(f"[ERROR] {e}")

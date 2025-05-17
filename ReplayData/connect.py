import socket
import serial
import threading
import time
import sys

# === CONFIGURATION ===
BAUDRATE = 250000
MULTICAST_GROUP = "239.255.50.10"
UDP_LISTEN_PORT = 5010
UDP_FORWARD_IP = "127.0.0.1"
UDP_FORWARD_PORT = 7778

# === Get COM port from arg or prompt ===
if len(sys.argv) >= 2:
    port_number = sys.argv[1]
else:
    port_number = input("Enter COM port number (e.g. 6 for COM6): ").strip()

COMPORT = f"COM{port_number}"

# === Init serial port ===
ser = serial.Serial()
ser.port = COMPORT
ser.baudrate = BAUDRATE
ser.bytesize = serial.EIGHTBITS
ser.parity = serial.PARITY_NONE
ser.stopbits = serial.STOPBITS_ONE
ser.dtr = True
ser.timeout = 0
ser.write_timeout = 0

try:
    ser.open()
    print(f"[OK] Opened {COMPORT} at {BAUDRATE} baud with DTR=ON")
except Exception as e:
    print(f"[ERROR] Could not open {COMPORT}: {e}")
    sys.exit(1)

# === RX socket: receive multicast ===
udp_rx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
udp_rx.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
udp_rx.bind(('', UDP_LISTEN_PORT))
mreq = socket.inet_aton(MULTICAST_GROUP) + socket.inet_aton('0.0.0.0')
udp_rx.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

# === TX socket: unicast to localhost ===
udp_tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

print(f"[OK] Listening on {MULTICAST_GROUP}:{UDP_LISTEN_PORT}, forwarding to {COMPORT}")

# === Rate counters ===
serial_out_count = 0
udp_in_count = 0

def rate_monitor():
    global serial_out_count, udp_in_count
    while True:
        time.sleep(1)
        print(f"[RATE] OUT: {serial_out_count} Hz, IN: {udp_in_count} Hz")
        serial_out_count = 0
        udp_in_count = 0

# === DCS-BIOS → ESP32 ===
def udp_to_serial():
    global udp_in_count
    while True:
        try:
            data, _ = udp_rx.recvfrom(4096)
            if data:
                ser.write(data)
                udp_in_count += 1
        except Exception as e:
            print(f"[ERROR] UDP read failed: {e}")
            break

# === ESP32 → DCS-BIOS Hub ===
def serial_to_udp():
    global serial_out_count
    while True:
        try:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                if data:
                    udp_tx.sendto(data, (UDP_FORWARD_IP, UDP_FORWARD_PORT))
                    serial_out_count += 1
                    print(f"[SERIAL→UDP] {data.decode(errors='replace').strip()}")
        except Exception as e:
            print(f"[ERROR] Serial read failed: {e}")
            break

# === Launch threads ===
threading.Thread(target=udp_to_serial, daemon=True).start()
threading.Thread(target=serial_to_udp, daemon=True).start()
threading.Thread(target=rate_monitor, daemon=True).start()

# === Keep alive ===
try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("\n[EXIT] Interrupted")
finally:
    ser.close()
    udp_rx.close()
    udp_tx.close()

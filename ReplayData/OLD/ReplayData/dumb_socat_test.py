import os
try:
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
except:
    pass

import socket
import serial
import logging
import time

# Configuration (adjust as needed)
COM_PORT = 'COM71'
BAUDRATE = 250000
UDP_PORT = 5010
MCAST_GROUP = '239.255.50.10'
BUFFER_SIZE = 8192

# Setup logging
logging.basicConfig(
    filename='dumb_socat_exact.log',
    level=logging.DEBUG,
    format='%(asctime)s %(levelname)s: %(message)s'
)

def open_serial(port, baudrate):
    # Open without toggling control lines: mimic socat
    ser = serial.Serial(
        port=port,
        baudrate=baudrate,
        timeout=0,
        dsrdtr=False,
        rtscts=False,
        xonxoff=False
    )
    logging.info(f"Opened serial port {port} at {baudrate}")
    print(f"Opened serial port {port}")
    return ser

def open_udp(port, mcast_group):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', port))
    mreq = socket.inet_aton(mcast_group) + socket.inet_aton('0.0.0.0')
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    logging.info(f"Listening on UDP {mcast_group}:{port}")
    print(f"Listening on UDP {mcast_group}:{port}")
    return sock

def main():
    ser = open_serial(COM_PORT, BAUDRATE)
    udp_sock = open_udp(UDP_PORT, MCAST_GROUP)
    try:
        while True:
            data, _ = udp_sock.recvfrom(BUFFER_SIZE)
            if data:
                logging.debug(f"Received {len(data)} bytes from UDP")
                # Forward in one large write to mimic socat
                ser.write(data)
                logging.debug(f"Wrote {len(data)} bytes to serial")
            time.sleep(0.001)
    except KeyboardInterrupt:
        print("Interrupted by user.")
    except Exception as e:
        logging.error("Error forwarding UDP to serial", exc_info=True)
        print(f"Error: {e}")
    finally:
        ser.close()
        udp_sock.close()
        logging.info("Shutdown complete")
        print("Shutdown complete")

if __name__ == '__main__':
    main()

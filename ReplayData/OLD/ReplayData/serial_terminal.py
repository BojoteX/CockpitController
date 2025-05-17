import os
# Change to script directory if running as .py
try:
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
except:
    pass

import sys
import socket
import serial
import logging

# Setup logging
LOG_FILE = 'serial_terminal.log'
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format='%(asctime)s %(levelname)s: %(message)s'
)

def prompt_settings():
    com_num = input("Enter COM port number (e.g., 2): ").strip()
    udp_port_str = input("Enter UDP listen port (e.g., 5005): ").strip()
    com_port = f"COM{com_num}" if com_num.isdigit() else com_num
    try:
        udp_port = int(udp_port_str)
    except ValueError:
        print(f"Invalid UDP port '{udp_port_str}'.")
        sys.exit(1)
    return com_port, udp_port

def open_serial(com_port, baudrate=250000):
    try:
        ser = serial.Serial(
            port=com_port,
            baudrate=baudrate,
            timeout=0,
            dsrdtr=False,
            rtscts=False,
            xonxoff=False
        )
        # Disable DTR explicitly
        ser.setDTR(False)
        logging.info(f"Opened {com_port} at {baudrate} baud with DTR disabled.")
        print(f"Opened {com_port} (DTR disabled).")
        return ser
    except Exception as e:
        logging.error(f"Failed to open {com_port}: {e}", exc_info=True)
        print(f"Error: Could not open {com_port}: {e}")
        sys.exit(1)

def main():
    com_port, udp_port = prompt_settings()
    ser = open_serial(com_port)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(('0.0.0.0', udp_port))
        logging.info(f"Listening for UDP on port {udp_port}.")
        print(f"Listening on UDP port {udp_port}. Forwarding to {com_port}.")
    except Exception as e:
        logging.error(f"Failed to bind UDP socket: {e}", exc_info=True)
        print(f"Error: Could not bind UDP port {udp_port}: {e}")
        ser.close()
        sys.exit(1)

    try:
        while True:
            try:
                data, _ = sock.recvfrom(4096)
                if data:
                    ser.write(data)
            except Exception:
                logging.error("Error forwarding UDP to serial", exc_info=True)
    except KeyboardInterrupt:
        print("\nInterrupted by user. Shutting down.")
    finally:
        ser.close()
        sock.close()
        logging.info("Closed serial port and UDP socket.")

if __name__ == '__main__':
    main()

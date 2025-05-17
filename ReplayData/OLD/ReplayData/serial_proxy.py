import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

import sys
# Guard against running the log file by mistake
if os.path.splitext(os.path.basename(__file__))[1] != '.py':
    print(f"Error: You are running '{os.path.basename(__file__)}', which is not the proxy script. Please run 'serial_proxy.py'.")
    sys.exit(1)

import serial
import time
import logging

# Setup logging
LOG_FILE = 'serial_proxy.log'
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.INFO,
    format='%(asctime)s %(levelname)s: %(message)s'
)

def prompt_ports():
    src_num = input("Enter source COM port number (e.g., 1): ").strip()
    dst_num = input("Enter destination COM port number (e.g., 69): ").strip()
    source = f"COM{src_num}" if src_num.isdigit() else src_num
    dest = f"COM{dst_num}" if dst_num.isdigit() else dst_num
    return source, dest

def open_serial(port, baudrate=250000):
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        logging.info(f"Opened {port} at {baudrate} baud.")
        print(f"Opened {port} successfully.")
        return ser
    except serial.SerialException as e:
        logging.error(f"Failed to open {port}: {e}")
        print(f"Error: Could not open {port}: {e}")
        raise

def main():
    try:
        source_port, dest_port = prompt_ports()
        src = open_serial(source_port)
        dst = open_serial(dest_port)
    except Exception:
        logging.critical("Could not open one or both serial ports. Exiting.")
        print("Critical: Could not open one or both serial ports. Check log for details.")
        return

    print(f"Proxying from {source_port} → {dest_port}")
    print("Press Ctrl+C to stop.")

    bytes_in = 0
    bytes_out = 0

    try:
        while True:
            try:
                size = src.in_waiting or 1
                data = src.read(size)
                if data:
                    bytes_in += len(data)
                    written = dst.write(data)
                    bytes_out += written if written is not None else len(data)
                    # Real-time status update on one line
                    sys.stdout.write(f"\rIN: {bytes_in} bytes | OUT: {bytes_out} bytes")
                    sys.stdout.flush()
            except serial.SerialException as e:
                logging.error(f"Serial I/O error: {e}")
                print(f"\nSerial I/O error: {e}")
            except Exception as e:
                logging.error("Unexpected error during read/write.", exc_info=True)
                print(f"\nUnexpected error: {e}")
            time.sleep(0.001)
    except KeyboardInterrupt:
        logging.info("Interrupted by user.")
        print("\nInterrupted by user. Shutting down.")
    finally:
        try:
            src.close()
            dst.close()
            logging.info("Closed serial ports.")
            print("Closed serial ports.")
        except Exception as e:
            logging.error(f"Error closing ports: {e}")
            print(f"Error closing ports: {e}")

if __name__ == '__main__':
    main()

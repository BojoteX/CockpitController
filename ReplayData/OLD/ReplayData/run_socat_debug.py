import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

import sys
import subprocess
import time
import logging
import serial

# Setup detailed logging
LOG_FILE = 'run_socat_debug.log'
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.DEBUG,
    format='%(asctime)s %(levelname)s: %(message)s'
)

def normalize_port(num):
    """Convert a digit or 'COMx' to Windows COM path, prefix '\\.\\' if >=10."""
    num = num.strip()
    if num.upper().startswith("COM"):
        n = num[3:]
    else:
        n = num
    if not n.isdigit():
        raise ValueError(f"Invalid COM port '{num}'")
    port_num = int(n)
    return f"\\.\\COM{port_num}" if port_num >= 10 else f"COM{port_num}"

def prompt_args():
    if len(sys.argv) > 1:
        com_num = sys.argv[1]
    else:
        com_num = input("Enter COM port number (e.g., 2): ")
    proto = (sys.argv[2].upper() if len(sys.argv) > 2 else input("Enter protocol [UDP/TCP] (default UDP): ").upper() or "UDP")
    if proto not in ("UDP", "TCP"):
        proto = "UDP"
    return normalize_port(com_num), proto

def apply_mode(port):
    cmd = f"mode {port} BAUD=250000 PARITY=N DATA=8 STOP=1 TO=off DTR=off"
    logging.info(f"Configuring port: {cmd}")
    print(f"Configuring: {cmd}")
    subprocess.run(cmd, shell=True)

def test_serial_write(port):
    logging.info(f"Testing write on {port}")
    print(f"Testing write on {port} ...")
    try:
        ser = serial.Serial(port, 250000, timeout=1, dsrdtr=False, rtscts=False, xonxoff=False)
        ser.setDTR(False)
        ser.write(b"TEST\n")
        ser.flush()
        ser.close()
        logging.info("Serial write test succeeded")
        print("Serial write test: SUCCESS")
    except Exception as e:
        logging.error(f"Serial write test failed: {e}", exc_info=True)
        print(f"Serial write test: FAILED ({e})")

def build_socat_cmd(proto, ttynum):
    base = "socat\\\socat -d -d"
    if proto == "UDP":
        return f"{base} UDP4-RECV:5010,ip-add-membership=239.255.50.10:0.0.0.0,reuseaddr!!udp-sendto:localhost:7778 /dev/ttyS{ttynum}"
    else:
        return f"{base} TCP4-CONNECT:127.0.0.1:7778 /dev/ttyS{ttynum}"

def run_socat(cmd):
    logging.info(f"Running socat: {cmd}")
    print(f"Starting socat: {cmd}")
    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    try:
        for line in proc.stdout:
            print(line, end="")
            logging.debug(line.strip())
    except KeyboardInterrupt:
        proc.terminate()
    proc.wait()
    logging.info(f"socat exited with code {proc.returncode}")

def main():
    try:
        com_port, proto = prompt_args()
    except ValueError as e:
        print(e)
        sys.exit(1)

    apply_mode(com_port)
    time.sleep(5)
    test_serial_write(com_port)

    # COMx -> ttyS(x-1)
    num = ''.join(filter(str.isdigit, com_port))
    tty_index = int(num) - 1
    socat_cmd = build_socat_cmd(proto, tty_index)
    run_socat(socat_cmd)

if __name__ == "__main__":
    main()

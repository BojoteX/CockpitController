import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

import subprocess
import sys
import logging

# Setup logging
LOG_FILE = 'socat_tester.log'
logging.basicConfig(
    filename=LOG_FILE,
    level=logging.DEBUG,
    format='%(asctime)s %(levelname)s: %(message)s'
)

def get_settings():
    com = input("Enter COM port number (e.g., 2): ").strip()
    proto = input("Enter protocol [UDP/TCP] (default UDP): ").strip().upper() or 'UDP'
    try:
        num = int(com)
    except ValueError:
        print(f"Invalid COM port number '{com}'.")
        sys.exit(1)
    tty = f"/dev/ttyS{num - 1}"
    return num, tty, proto

def build_commands(tty, proto):
    base = "socat\\socat"
    debug = "-d -d -d -d -x"
    cmd_base = f"{base} {debug}"
    commands = []
    if proto == 'UDP':
        spec = "UDP4-RECV:5010,ip-add-membership=239.255.50.10:0.0.0.0,reuseaddr"
        for opts in ["", ",raw", ",raw,echo=0", ",raw,echo=0,crtscts=0", ",raw,echo=0,crtscts=0,dsrdtr=0"]:
            cmd = f"{cmd_base} {spec}!!udp-sendto:localhost:7778 {tty}{opts}"
            commands.append((f"UDP{opts or ''}", cmd))
    else:
        spec = "TCP4-CONNECT:127.0.0.1:7778"
        for opts in ["", ",raw", ",raw,echo=0", ",raw,echo=0,crtscts=0", ",raw,echo=0,crtscts=0,dsrdtr=0"]:
            cmd = f"{cmd_base} {spec} {tty}{opts}"
            commands.append((f"TCP{opts or ''}", cmd))
    return commands

def main():
    num, tty, proto = get_settings()
    commands = build_commands(tty, proto)
    print("Available socat test commands:")
    for i, (name, cmd) in enumerate(commands):
        print(f"{i}: {name} -> {cmd}")
    sel = input(f"Select test index [0-{len(commands)-1}]: ").strip()
    if not sel.isdigit() or int(sel) not in range(len(commands)):
        print("Invalid selection.")
        sys.exit(1)
    idx = int(sel)
    name, cmd = commands[idx]
    print(f"Running test '{name}'...")
    logging.info(f"Starting test {name}: {cmd}")

    proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    try:
        for line in proc.stdout:
            print(line, end='')
            logging.debug(line.strip())
    except KeyboardInterrupt:
        print("\nTest interrupted by user.")
        proc.terminate()
    ret = proc.wait()
    print(f"Process exited with code {ret}")
    logging.info(f"Test '{name}' exited with code {ret}")

if __name__ == '__main__':
    main()

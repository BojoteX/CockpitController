import os
os.chdir(os.path.dirname(os.path.abspath(__file__)))

import sys
import subprocess
import time

def main():
    # 1) Get COM port number
    if len(sys.argv) > 1:
        port_arg = sys.argv[1]
    else:
        port_arg = input("Enter COM port number (e.g., 1): ").strip()
    # Strip "COM" prefix if present
    num = port_arg[3:] if port_arg.upper().startswith("COM") else port_arg
    try:
        ttynum = int(num) - 1
    except ValueError:
        print(f"Invalid COM port '{port_arg}'. Exiting.")
        sys.exit(1)

    # 2) Protocol selection (default UDP)
    proto = input("Enter protocol [UDP/TCP] (default UDP): ").strip().upper() or "UDP"
    if proto not in ("UDP", "TCP"):
        print(f"Invalid protocol '{proto}', defaulting to UDP.")
        proto = "UDP"

    verbose = "-v"

    # 3) Apply mode settings (identical to .cmd script)
    mode_cmd = f"mode COM{num} BAUD=250000 PARITY=N DATA=8 STOP=1 TO=off DTR=off"
    print(f"Running: {mode_cmd}")
    subprocess.run(mode_cmd, shell=True)
    time.sleep(5)

    # 4) Build socat command line exactly as in the .cmd
    if proto == "UDP":
        socat_cmd = (
            f"socat\\socat {verbose} "
            "UDP4-RECV:5010,ip-add-membership=239.255.50.10:0.0.0.0,"
            "reuseaddr!!udp-sendto:localhost:7778 "
            f"/dev/ttyS{ttynum}"
        )
    else:
        socat_cmd = (
            f"socat\\socat {verbose} "
            "TCP4-CONNECT:127.0.0.1:7778 "
            f"/dev/ttyS{ttynum}"
        )

    print(f"Executing: {socat_cmd}")
    subprocess.run(socat_cmd, shell=True)

if __name__ == "__main__":
    main()

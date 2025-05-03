import socket
import ctypes
import time
import logging
from ctypes import wintypes

# ---------------- Configuration ----------------
COM_PORT     = 'COM71'               # the COM port to open
BAUDRATE     = 250000               # baud rate
UDP_PORT     = 5010                 # local UDP port to bind
MCAST_GROUP  = '239.255.50.10'      # multicast group
BUFFER_SIZE  = 8192                 # max UDP packet size to read
# -----------------------------------------------

# Setup logging to a file
logging.basicConfig(
    filename='dumb_socat_lowlevel.log',
    level=logging.DEBUG,
    format='%(asctime)s %(levelname)s: %(message)s'
)

# Windows API constants
GENERIC_READ             = 0x80000000
GENERIC_WRITE            = 0x40000000
OPEN_EXISTING            = 3
FILE_ATTRIBUTE_NORMAL    = 0x80
SETDTR                   = 0x1
SETRTS                   = 0x2

# Load kernel32 functions
kernel32 = ctypes.WinDLL('kernel32', use_last_error=True)
CreateFileA = kernel32.CreateFileA
CreateFileA.argtypes = [
    wintypes.LPCSTR, wintypes.DWORD, wintypes.DWORD,
    wintypes.LPVOID, wintypes.DWORD, wintypes.DWORD, wintypes.HANDLE
]
CreateFileA.restype = wintypes.HANDLE

WriteFile = kernel32.WriteFile
WriteFile.argtypes = [
    wintypes.HANDLE, wintypes.LPCVOID, wintypes.DWORD,
    ctypes.POINTER(wintypes.DWORD), wintypes.LPVOID
]
WriteFile.restype = wintypes.BOOL

EscapeCommFunction = kernel32.EscapeCommFunction
EscapeCommFunction.argtypes = [wintypes.HANDLE, wintypes.DWORD]
EscapeCommFunction.restype = wintypes.BOOL

CloseHandle = kernel32.CloseHandle

def open_serial_raw(com_port):
    """Open COM port with raw Win32 CreateFile (no overlapped, no share flags)"""
    # Build null-terminated ASCII path: "\\\\.\\COM71"
    path = f"\\\\.\\{com_port}".encode('ascii') + b'\x00'
    h = CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        0,                # no share
        None,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        None
    )
    if h == wintypes.HANDLE(-1).value:
        err = ctypes.get_last_error()
        raise OSError(f"CreateFile failed on {com_port}, error code {err}")
    logging.info(f"Opened raw handle to {com_port}: {h:#x}")
    print(f"Opened raw handle to {com_port}")
    # Mimic socat defaults: assert DTR and RTS on open
    EscapeCommFunction(h, SETDTR)
    EscapeCommFunction(h, SETRTS)
    logging.info("Asserted DTR and RTS high")
    return h

def open_udp(port, group):
    """Bind a UDP socket and join the multicast group."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', port))
    mreq = socket.inet_aton(group) + socket.inet_aton('0.0.0.0')
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    logging.info(f"Listening UDP {group}:{port}")
    print(f"Listening UDP {group}:{port}")
    return sock

def main():
    com_handle = open_serial_raw(COM_PORT)
    udp_sock   = open_udp(UDP_PORT, MCAST_GROUP)

    try:
        while True:
            data, _ = udp_sock.recvfrom(BUFFER_SIZE)
            if not data:
                time.sleep(0.001)
                continue

            length = len(data)
            logging.debug(f"Received {length} bytes from UDP")
            # Do a single large WriteFile call
            written = wintypes.DWORD(0)
            success = WriteFile(com_handle, data, length, ctypes.byref(written), None)
            if not success:
                err = ctypes.get_last_error()
                logging.error(f"WriteFile failed: error {err}")
                print(f"WriteFile failed: error {err}")
                break
            logging.debug(f"Wrote {written.value} bytes to serial")

    except KeyboardInterrupt:
        print("Interrupted by user")

    finally:
        CloseHandle(com_handle)
        udp_sock.close()
        logging.info("Shutdown complete")
        print("Shutdown complete")

if __name__ == '__main__':
    main()

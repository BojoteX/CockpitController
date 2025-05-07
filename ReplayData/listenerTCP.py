import socket
import select
import re

# === Configuration ===
TCP_PORT = 4210
BUFFER_SIZE = 1024
STRIP_ANSI = False  # set to True to remove color codes

ansi_escape = re.compile(r'\x1B[@-_][0-?]*[ -/]*[@-~]')

# Show local IP
hostname = socket.gethostname()
local_ip = socket.gethostbyname(hostname)
print(f"[✓] TCP Debug Server listening on {local_ip}:{TCP_PORT}...\n")

# Set up TCP listener
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('', TCP_PORT))
server.listen(1)
server.setblocking(False)

inputs = [server]
client_buffers = {}

def sanitize(msg):
    msg = msg.decode('utf-8', errors='ignore')
    if STRIP_ANSI:
        msg = ansi_escape.sub('', msg)
    return msg

try:
    while True:
        readable, _, _ = select.select(inputs, [], [], 0.1)
        for sock in readable:
            if sock is server:
                conn, addr = server.accept()
                print(f"[🔌] ESP32 connected from {addr[0]}")
                conn.setblocking(False)
                inputs.append(conn)
                client_buffers[conn] = b''
            else:
                data = sock.recv(BUFFER_SIZE)
                if not data:
                    print(f"[❌] Disconnected from {sock.getpeername()[0]}")
                    inputs.remove(sock)
                    del client_buffers[sock]
                    sock.close()
                    continue

                # Append to buffer
                client_buffers[sock] += data
                while b'\n' in client_buffers[sock]:
                    line, remainder = client_buffers[sock].split(b'\n', 1)
                    client_buffers[sock] = remainder
                    clean = sanitize(line).strip()
                    print(f"[{sock.getpeername()[0]}] {clean}")

except KeyboardInterrupt:
    print("\n[✋] Stopped by user.")
finally:
    for s in inputs:
        s.close()
    server.close()
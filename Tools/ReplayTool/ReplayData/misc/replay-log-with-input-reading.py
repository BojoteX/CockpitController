import serial
import time
import threading
import json

def read_messages(file_path):
    with open(file_path, "r") as f:
        data = json.load(f)
    return [(item['timing'], bytes.fromhex(item['data'])) for item in data]

def read_from_device(ser):
    while True:
        if ser.in_waiting > 0:
            print(ser.read(ser.in_waiting).decode(errors='replace'), end='')

if __name__ == "__main__":
    com_port_number = input("Enter a COM Port Number: ")
    com_port = f'COM{com_port_number}'

    messages = read_messages("dcsbios_data.json")

    with serial.Serial(com_port, 250000, timeout=0) as ser:
        threading.Thread(target=read_from_device, args=(ser,), daemon=True).start()

        first_loop = True
        while True:
            loop_messages = messages if first_loop else messages[1:]

            for elapsed_time, data in loop_messages:
                try:
                    elapsed_time_float = float(elapsed_time)
                except ValueError:
                    print(f"Invalid timing: {elapsed_time}")
                    continue

                ser.write(data)
                time.sleep(elapsed_time_float)

            first_loop = False

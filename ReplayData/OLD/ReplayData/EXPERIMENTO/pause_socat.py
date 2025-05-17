import serial
import time

# Configuration
COM_PORT       = 'COM2'    # the paired port where we drive RTS
BAUDRATE       = 250000    # must match socat
PAUSE_AFTER    = 10        # seconds to wait before pausing
PAUSE_DURATION = 5         # how long to hold pause (RTS low)
SLEEP_INT      = 0.05      # read loop interval

def main():
    # Open COM2 with no auto‐handshake
    ser = serial.Serial(
        port=COM_PORT,
        baudrate=BAUDRATE,
        timeout=0,
        rtscts=False,    # we’ll manually toggle RTS
        dsrdtr=False,
        xonxoff=False
    )
    print(f"Opened {COM_PORT} @ {BAUDRATE} baud")

    start = time.time()
    paused = False

    try:
        while True:
            # just read so we can log data if you like
            data = ser.read(1024)
            if data:
                print(f"[{time.time()-start:6.2f}s] Got {len(data)} bytes")

            elapsed = time.time() - start

            # After PAUSE_AFTER, pull RTS low => CTS=0 on COM1 => socat pauses
            if not paused and elapsed >= PAUSE_AFTER:
                print(f"[{elapsed:6.2f}s] ➔ PAUSE (RTS LOW)")
                ser.setRTS(False)
                pause_start = time.time()
                paused = True

            # After PAUSE_DURATION, set RTS high => CTS=1 => socat resumes
            if paused and (time.time() - pause_start) >= PAUSE_DURATION:
                print(f"[{time.time()-start:6.2f}s] ➔ RESUME (RTS HIGH)")
                ser.setRTS(True)
                break

            time.sleep(SLEEP_INT)

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        print("Done.")

if __name__ == '__main__':
    main()

# DCS-BIOS Bridge Auto-Generator

This tool generates all runtime data tables required for an embedded ESP32-based DCS-BIOS controller system. It parses the official `FA-18C_hornet.json` (included in the [DCS-BIOS Skunkworks](https://github.com/DCS-Skunkworks/dcs-bios) release) and outputs:

- `DCSBIOSBridgeData.h` — selector/analog mapping, output table, address hashing, command tracking  
- `InputMapping.h` — per-input configuration (pin, label, command, HID ID, override)  
- `LEDMapping.h` — output mapping to physical LED devices (GPIO, PCA9555, WS2812, etc.)

All generated structures are static, flash-resident, and zero-heap — engineered for real-time embedded performance.

---

## ✅ Setup Instructions

1. Place `generate_data.py` inside your firmware root folder.
2. Ensure the correct aircraft JSON file (e.g. `FA-18C_hornet.json`) is present.
3. Run:

```bash
python3 generate_data.py
```

This updates the 3 header files with the selected panel data.

---

## 🔍 Target Panel Selection (`target_objects`)

By default, the generator includes only selected physical panels:

```python
target_objects = {
    'Fire Systems',
    'Master Arm Panel',
    'Caution Light Panel',
    ...
}
```

To include **all** panels from the aircraft:

```python
PROCESS_ALL = True
```

You can also customize the panel list for your own cockpit.

---

## 🧠 Output File Breakdown

### `DCSBIOSBridgeData.h`

- `DcsOutputTable[]`: maps DCS memory regions to label + type
- `SelectorMap[]`: maps selector/button/analog labels to DCS command/values
- `commandHistory[]`: throttle + keep-alive tracking (per label)
- `dcsAddressHashTable[]`: fast O(1) address→output table lookup

### `InputMapping.h`

Each input is mapped with:
- Hardware source (`GPIO`, `PCA9555`, etc.)
- Port, bit, HID usage ID
- Override DCS command/argument
- Input type (`selector`, `momentary`, `analog`)
- Exclusive group ID (for switches with mutual exclusion)

Edits to physical wiring info (ports, bits, HID ID) are preserved between runs.

### `LEDMapping.h`

- Maps `CT_LED` and `CT_ANALOG` outputs to physical devices
- Supported device types:
  - `GPIO`, `PCA9555`, `TM1637`, `GN1640T`, `WS2812`
- Unmapped items are marked `DEVICE_NONE` and skipped safely at runtime

---

## ⚙️ Runtime Integration

- `findInputByLabel(label)` and `findLED(label)` provide zero-cost lookups
- `sendDCSBIOSCommand(label, value)` handles all DCS-BIOS output
- All HID and DCS outputs are throttled consistently
- LED updates are deferred and flushed per DCS frame (via `onLedChange()`)

---

## 🛡️ Performance & Safety

- No heap use — all memory is `static const`
- Hash tables auto-sized using `next_prime()`
- Compile-time enforcement for all mappings
- Generator aborts on hash collision overflows
- Struct fields explicitly sized (`uint16_t`, `uint8_t`, etc.)

---

## 📦 Aircraft Support

To adapt for another DCS aircraft:

1. Replace `FA-18C_hornet.json` with your aircraft’s export JSON
2. Adjust `target_objects` to match available panel names
3. Run the generator
4. Recompile your firmware — no other changes needed

---

## ✅ Verified Compatibility

This generator is used in production as part of:

- [CockpitController (ESP32-S2)](https://github.com/Bojote/CockpitController)
- Arduino Core 2.x / 3.x with TinyUSB composite
- F/A-18C Hornet + other DCS aircraft modules via Skunkworks JSON
- USB HID + DCS-BIOS hybrid systems

---

## 💬 Contact & Contributions

Maintained by [@BojoteX](https://github.com/BojoteX).  
Pull requests welcome to add panel mappings, enhance tooling, or support additional aircraft.

---

**✈️ Fly safe. Build smarter.**

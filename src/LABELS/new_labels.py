import json
import re

# ------- CONFIGURATION -------
# Set the JSON file containing control definitions (addresses, masks, etc.)
json_file = "FA-18C_hornet.json"
# Set the Lua file containing the human-readable control definitions (state descriptions)
lua_file = "FA-18C_hornet.lua"
# Specify which top-level objects from the JSON file to process.
target_objects = [
    "Caution Light Panel",
    "Dispenser/EMC Panel",
    "LH Advisory Panel",
    "Left Engine Fire Warning Extinguisher Light",
    "Lock Shoot Lights",
    "Map Gain/Spin Recovery Panel",
    "Master Arm Panel",
    "Master Caution Light",
    "RH Advisory Panel",
    "Right Engine Fire Warning Extinguisher Light"
]
# ------- END CONFIGURATION -------

# ====================
# Helper function definitions
# ====================

def generate_led_callback(identifier):
    return f'void onLEDChange_{identifier}(unsigned int newValue) {{ setLED("{identifier}", newValue); }}'

def generate_led_buffer(identifier, address, mask, shift_by):
    return f'DcsBios::IntegerBuffer {identifier}_Buffer({hex(address)}, {hex(mask)}, {shift_by}, onLEDChange_{identifier});'

def generate_selector_callback(identifier):
    return f'void onSelectorChange_{identifier}(unsigned int newValue) {{ setSelector("{identifier}", newValue); }}'

def generate_selector_buffer(identifier, address, mask, shift_by):
    return f'DcsBios::IntegerBuffer {identifier}_Buffer({hex(address)}, {hex(mask)}, {shift_by}, onSelectorChange_{identifier});'

def parse_lua_for_selector_states(lua_filename):
    mapping = {}
    pattern = r'FA_18C_hornet:define(?:3PosTumb|Tumb|ToggleSwitch(?:ToggleOnly)?)\(\s*"([^"]+)"\s*,[^,]+,[^,]+,[^,]+,[^,]+,\s*"([^"]+)"\s*\)'
    with open(lua_filename, "r") as f:
        lua_text = f.read()
    for match in re.finditer(pattern, lua_text):
        identifier, desc = match.groups()
        desc = desc.strip()
        if "," in desc:
            title, state_str = desc.split(",", 1)
            states = [s.strip() for s in state_str.split("/") if s.strip()]
            mapping[identifier] = (states, desc)
        else:
            mapping[identifier] = ([], desc)
    return mapping

# ====================
# Main script execution
# ====================

def main():
    with open(json_file, "r") as f:
        data = json.load(f)

    lua_state_meanings = parse_lua_for_selector_states(lua_file)

    led_callbacks = []
    led_buffers = []
    selector_callbacks = []
    selector_buffers = []
    selector_options_info = []

    for obj_name, controls in data.items():
        if obj_name not in target_objects:
            continue

        for key, details in controls.items():
            control_type = details.get("control_type", "").lower()
            identifier = details.get("identifier", key)
            outputs = details.get("outputs", [])
            if not outputs:
                continue
            output = outputs[0]
            address = output.get("address")
            mask = output.get("mask")
            shift_by = output.get("shift_by")

            if control_type == "led":
                led_callbacks.append(generate_led_callback(identifier))
                led_buffers.append(generate_led_buffer(identifier, address, mask, shift_by))
            elif control_type == "selector":
                inputs = details.get("inputs", [])
                selector_callbacks.append(generate_selector_callback(identifier))
                selector_buffers.append(generate_selector_buffer(identifier, address, mask, shift_by))
                for inp in inputs:
                    if inp.get("interface") == "set_state":
                        max_val = inp.get("max_value")
                        if max_val is not None and max_val > 1:
                            state_list, full_desc = lua_state_meanings.get(identifier, ([str(i) for i in range(max_val + 1)], None))
                            selector_options_info.append((identifier, max_val, state_list, full_desc))
                        break

    header = (
        "/*\n"
        " * DCSCallbacks.cpp\n"
        " * Auto-generated callbacks and DCS-BIOS buffer declarations\n"
        " */\n\n"
        "#include \"DcsBios.h\"\n\n"
    )

    led_section = "// -------- LED Callbacks --------\n\n" + "\n".join(led_callbacks)
    led_section += "\n\n// -------- LED DCS-BIOS Buffer Declarations --------\n\n" + "\n".join(led_buffers)

    selector_section = "\n\n// -------- Selector Callbacks --------\n\n" + "\n".join(selector_callbacks)
    selector_section += "\n\n// -------- Selector DCS-BIOS Buffer Declarations --------\n\n" + "\n".join(selector_buffers)

    options_section = "\n\n/*\n * -------- Enhanced Selector Option Information (from Lua Definitions) --------\n"
    for identifier, max_val, state_list, full_desc in selector_options_info:
        options_section += f" *\n * {identifier}:\n *   Positions: {', '.join(f'{i} = {state}' for i, state in enumerate(state_list))}\n"
        if full_desc:
            options_section += f" *   Lua Definition: {full_desc}\n"
    options_section += " */\n"

    file_content = header + led_section + selector_section + options_section

    with open("DCSCallbacks.cpp", "w") as out_file:
        out_file.write(file_content)

if __name__ == "__main__":
    main()

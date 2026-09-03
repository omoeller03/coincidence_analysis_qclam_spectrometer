import yaml
import re

fIn = "driftTime_corrections.txt"

def parse_config_line(line):
    line = line.strip()

    # Kommentare ignorieren
    if not line or line.startswith("#") or line.startswith("//"):
        return None, None, None

    # invert=true
    m = re.match(r"invert\s*=\s*(\w+)", line)
    if m:
        return "invert", m.group(1).lower() == "true", None

    # GlobalOffset=0
    m = re.match(r"GlobalOffset\s*=\s*([-\d\.]+)", line)
    if m:
        return "global", float(m.group(1)), None

    # TDCOffset-<tdc>=<offset>
    m = re.match(r"TDCOffset-(\d+)\s*=\s*([-\d\.]+)", line)
    if m:
        return "tdc", (int(m.group(1)), float(m.group(2))), None

    # TDCChannelOffset-<tdc>,<ch>=<offset>
    m = re.match(r"TDCChannelOffset-(\d+),(\d+)\s*=\s*([-\d\.]+)", line)
    if m:
        return "tdcChannel", (int(m.group(1)), int(m.group(2)), float(m.group(3))), None

    # LayerOffset-<layer>=<offset>
    m = re.match(r"LayerOffset-(\d+)\s*=\s*([-\d\.]+)", line)
    if m:
        return "layer", (int(m.group(1)), float(m.group(2))), None

    # WireOffset-<layer>,<wire>=<offset>
    m = re.match(r"WireOffset-(\d+),0*([0-9]+)\s*=\s*([-\d\.]+)", line)
    if m:
        return "wire", (int(m.group(1)), int(m.group(2)), float(m.group(3))), None

    return None, None, None


def convert_offsets(input_file, output_file):
    data = {
        "offsets": {
            "invert": True,
            "global": 0,
            "tdc": {},
            "tdcChannel": {},
            "layer": {},
            "wire": {}
        }
    }

    with open(input_file, "r") as f:
        for line in f:
            key, value, _ = parse_config_line(line)
            if key is None:
                continue

            if key == "invert":
                data["offsets"]["invert"] = value

            elif key == "global":
                data["offsets"]["global"] = value

            elif key == "tdc":
                tdc, offset = value
                data["offsets"]["tdc"][tdc] = offset

            elif key == "tdcChannel":
                tdc, ch, offset = value
                data["offsets"]["tdcChannel"].setdefault(tdc, {})
                data["offsets"]["tdcChannel"][tdc][ch] = offset

            elif key == "layer":
                layer, offset = value
                data["offsets"]["layer"][layer] = offset

            elif key == "wire":
                layer, wire, offset = value
                data["offsets"]["wire"].setdefault(layer, {})
                data["offsets"]["wire"][layer][wire] = offset

    with open(output_file, "w") as f:
        yaml.dump(data, f, sort_keys=True)

    print("Konvertierung abgeschlossen.")


if __name__ == "__main__":
    convert_offsets(fIn, "offsets.yaml")

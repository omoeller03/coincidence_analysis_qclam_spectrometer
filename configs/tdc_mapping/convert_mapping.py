import yaml
import re

fIn = "tdc_mapping_4x1.txt"

# YAML soll Inline-Dicts für die Channel-Einträge verwenden
class InlineDict(dict):
    pass

def inline_dict_representer(dumper, data):
    return dumper.represent_mapping('tag:yaml.org,2002:map', data, flow_style=True)

yaml.add_representer(InlineDict, inline_dict_representer)


def parse_line(line):
    """
    Erwartetes Format:
    <tdc>,<channel>=<ebene>,<draht>-<activate>
    activate ist optional
    """
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    m = re.match(r"(\d+),(\d+)\s*=\s*(\d+),(\d+)(?:-(\S+))?", line)
    if not m:
        return None

    tdc = int(m.group(1))
    channel = int(m.group(2))
    layer = int(m.group(3))
    wire = int(m.group(4))
    activate_raw = m.group(5)

    # Aktivierungslogik
    if activate_raw is None:
        active = True
    else:
        active = not (activate_raw in ["0", "false", "False"])

    return tdc, channel, layer, wire, active


def convert_config(input_file, output_file):
    mapping = {}

    with open(input_file, "r") as f:
        for line in f:
            parsed = parse_line(line)
            if parsed is None:
                continue

            tdc, channel, layer, wire, active = parsed

            tdc_key = f"tdc{tdc}"
            if tdc_key not in mapping:
                mapping[tdc_key] = {}

            entry = InlineDict({"layer": layer, "wire": wire, "active": active})
            #if not active:
            #    entry["active"] = False

            mapping[tdc_key][channel] = entry

    with open(output_file, "w") as f:
        yaml.dump({"mapping": mapping}, f, sort_keys=True)


if __name__ == "__main__":
    convert_config(fIn, "new_config.yaml")
    print("Konvertierung abgeschlossen.")

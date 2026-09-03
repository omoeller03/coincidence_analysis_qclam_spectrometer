import csv

layerID = "1"
title = "WireOffset-"

inputF="time_offsets_U.txt"
output="time_offsets_U_out.txt"

data = []

with open(inputF, encoding="utf-8") as f:
    reader = csv.reader(f, delimiter='\t')


    for line in reader:
        wire_number = line[0]
        offset = line[1]

        new_line = title + layerID + "," + wire_number + "=" + offset

        data.append(new_line)


with open(output, "w", encoding="utf-8") as out:
    #writer = csv.writer(out)
    for val in data:
        out.write(val + "\n")

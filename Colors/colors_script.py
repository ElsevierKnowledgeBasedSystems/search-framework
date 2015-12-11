#!/usr/bin/python3
import fileinput

def myprint(msg):
    print(msg, end="", flush=True)

print("#ifndef COLORS")
print("#define COLORS")
print()
print("#include <vector>")
print()
print("// The source of colors: http://xkcd.com/color/rgb/")
names = ["NOVAL"]; values = ["0xffffff"]
for line in fileinput.input():
    if line[0] == "#": continue
    line = line.replace('\'', '')
    line = line.replace('/', ' over ')
    parts = line.split()
    names += ["_".join(parts[:-1]).upper()]
    values += ["0x"+parts[-1][1:]]

print("enum class Color { " + ", ".join(names) + " };")
print()
print("std::vector<int> RGBValues { " + ", ".join(values) + " };")
print()
print("#endif")

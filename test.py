#!/usr/bin/env python3
from os import system
from sys import argv
from secrets import token_hex
from json import loads, dumps

flags = []
if len(argv) <= 1:
    flags.extend(["pm", "m", "r", "t"])
else:
    flags.extend(argv[1:])

projectname = "a"
with open("./premake5.lua", "r") as f:
    lines = [i for i in f.read().split("\n") if i.startswith("project(\"")]
    projectname = lines[0].split("\"")[1]

if "ui" in flags:
    system("git submodule update --init --recursive --remote")
if "u" in flags:
    system("git submodule update --recursive --remote")
if "pm" in flags:
    system("premake5 gmake2")
if "m" in flags:
    #system("rm -r obj/*")
    system("rm ./bin/Debug/"+projectname)
    system("make config=debug")
    with open("./config.json", "r+") as f:
        cfg = loads(f.read())
        cfg["captcha_secret"] = token_hex(1024)
        f.seek(0)
        f.write(dumps(cfg, indent = 4))
if "r" in flags:
    print("running~~~")
    system("./bin/Debug/"+projectname)
if "d" in flags:
    system("gdb ./bin/Debug/"+projectname)
if "pr" in flags:
    system("valgrind --tool=callgrind bin/Debug/"+projectname+" ./sample/")
    system("kcachegrind ./callgrind*")
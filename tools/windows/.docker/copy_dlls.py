#!/usr/bin/env python3

import glob
import os
import shutil
import subprocess

NOTFOUND = []


def dll_names(binary):
    out = subprocess.check_output(
        [
            "sh",
            "-c",
            "x86_64-w64-mingw32-objdump -p %s | grep dll | awk '{print $3}'" % binary,
        ]
    )
    return [x for x in out.decode("utf-8").split("\n") if x and "dll" in x]


def copy_dlls(binary):
    print("Copying DLLs for {}".format(binary))
    res = dll_names(binary) + ["libssl-1_1-x64.dll"]
    for dll in res:
        if os.path.exists(dll) or dll in NOTFOUND:
            continue
        dll_files = list(
            glob.iglob("/usr/x86_64-w64-mingw32/**/{}".format(dll), recursive=True)
        )
        if not dll_files:
            print("File {} not found".format(dll))
            NOTFOUND.append(dll)
        else:
            print(dll_files)
            shutil.copy(dll_files[0], "./{}".format(dll))
            copy_dlls(dll)


copy_dlls("valeronoi.exe")

#!/usr/bin/env python3

"""
This tools copies the required DLLs

Alternative solution to the options mentioned on https://www.msys2.org/wiki/Distributing/
"""

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
            "objdump -p %s | grep dll | awk '{print $3}'" % binary,
        ]
    )
    return [x for x in out.decode("utf-8").split("\n") if x and "dll" in x]


def copy_dlls(binary, target="./"):
    print("Copying DLLs for {}".format(binary))
    res = dll_names(binary) + ["libssl-1_1-x64.dll"]
    for dll in res:
        if os.path.exists(dll) or dll in NOTFOUND:
            continue
        dll_files = list(glob.iglob("C:/msys2/**/{}".format(dll), recursive=True))
        if not dll_files:
            print("File {} not found".format(dll))
            NOTFOUND.append(dll)
        else:
            print(dll_files)
            shutil.copy(dll_files[0], "{}{}".format(target, dll))
            copy_dlls(dll)


def copy_additional_dlls(dir, dlls):
    os.makedirs(dir)
    # Qt 6.2 requires more magic for TLS
    for dll in dlls:
        dll_files = list(glob.iglob("C:/msys2/**/{}".format(dll), recursive=True))
        if not dll_files:
            print("File {} not found".format(dll))
            continue
        print(dll_files)
        shutil.copy(dll_files[0], "{}/{}".format(dir, dll))


copy_dlls("valeronoi.exe")
copy_additional_dlls(
    "tls", ["qcertonlybackend.dll", "qopensslbackend.dll", "qschannelbackend.dll"]
)
copy_additional_dlls("styles", ["qwindowsvistastyle.dll"])
copy_additional_dlls(
    "platforms", ["qdirect2d.dll", "qminimal.dll", "qoffscreen.dll", "qwindows.dll"]
)

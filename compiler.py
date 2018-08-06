#!/usr/bin/env python

import os
import re
import shutil
import subprocess
import sys
import tempfile

args = []
tmpdirs = {}  # {'.a': 'tmpdir/'}
bcs = set()
for arg in sys.argv[1:]:
    if re.match(r'CMakeFiles/fuzz-.*\.dir/__/__/.*\.c\.bc', arg):
        continue  # pni_sniff_header has internal linking in normal compilation, here we would have duplicate symbol
    if arg.endswith(".a"):
        if arg in tmpdirs:
            d = tmpdirs[arg]
        else:
            d = tempfile.mkdtemp(os.path.basename(arg))
            subprocess.check_call(executable="ar", shell=True, args=['-x', os.path.abspath(arg)], cwd=d)
            tmpdirs[arg] = d
            # add .bc files only if we see the .a for the first time, clang will fail on duplicities
            new_bcs = [b for b in os.listdir(d) if b not in bcs]
            args.extend(os.path.join(d, f) for f in new_bcs)
            print("new bcs", set(new_bcs))
            bcs.update(new_bcs)
            print("bcs", bcs)
    else:
        args.append(arg)
print(args)
argv0 = 'divine'
try:
    subprocess.check_call(executable=argv0, args=[argv0, 'cc'] + args)
finally:
    for d in tmpdirs.values():
        shutil.rmtree(d)


# clang -z muldefs c-condition-tests.bc glibc.bc
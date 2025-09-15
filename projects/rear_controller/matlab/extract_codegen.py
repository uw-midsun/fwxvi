import os
import shutil
from pathlib import Path

BASE_DIR = Path("./simulink")
TARGET_DIR = BASE_DIR.parent.parent / "codegen"

INC_DIR = TARGET_DIR / "inc"
SRC_DIR = TARGET_DIR / "src"

INC_DIR.mkdir(parents=True, exist_ok=True)
SRC_DIR.mkdir(parents=True, exist_ok=True)

VALID_EXTS = {".h", ".hpp", ".c", ".cc", ".cpp"}

moved_files = []

for root, _, files in os.walk(BASE_DIR):
    for file in files:
        ext = Path(file).suffix.lower()
        if ext in VALID_EXTS:
            src = Path(root) / file
            if ext == ".h" or ext == ".hpp":
                dst = INC_DIR / file
            else:  # .c, .cc, .cpp
                dst = SRC_DIR / file
            shutil.move(str(src), str(dst))
            moved_files.append(dst)


print(f"Moved {len(moved_files)} files into matlab/codegen")

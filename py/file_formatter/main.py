# @defgroup file_formatter
#  This is a python package for file_formatter
#
# @file    file_formatter.py
#  @date    2025-04-20
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Core module for file_formatter
#
#  @details
#
#  @ingroup file_formatter

from pathlib import Path

import argparse

from cstyle_file_formatter import CStyleFileFormatter
from formatter_config import *

cstyle_file_formatter = CStyleFileFormatter()


def process_directory(directory_path: Path):
    """
    @brief Processes all Python and header files in the given directory and its subdirectories
    @param directory_path (Path) The root directory to process
    """
    if not directory_path.is_dir():
        print(f"Error: '{directory_path}' is not a valid directory.")
        return

    for filepath in directory_path.rglob("*"):
        try:
            rel_path = filepath.relative_to(directory_path).as_posix()
        except ValueError:
            continue  # Skip if filepath isn't under directory_path

        if any(rel_path.startswith(ignore) for ignore in IGNORE_DIRS):
            continue  # Skip files in ignored directories

        if filepath.is_file() and filepath.suffix in [".py", ".h", ".c"]:
            # if filepath.suffix == ".py":
            #     # header = create_python_header(filepath.name)
            #     # update_header(filepath, header)

            if filepath.suffix in [".h", ".c"]:
                cstyle_file_formatter.format_file(filepath)


def main():
    """
    @brief Main function for file_formatter
    @details Parser expects an entry for project folder orr if left empty, it will autoformat every file
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("--project", help="Path to the 'project' directory.")
    parser.add_argument("--smoke", help="Path to the 'smoke' directory.")
    parser.add_argument("--py", help="Path to the 'py' directory.")

    args = parser.parse_args()

    if args.project is not None:
        process_directory(Path.cwd() / Path("projects") / Path(args.project))

    if args.smoke is not None:
        process_directory(Path.cwd() / Path("smoke") / Path(args.smoke))

    if args.py is not None:
        process_directory(Path.cwd() / Path("py") / Path(args.py))

    if args.project is None and args.smoke is None and args.py is None:
        process_directory(Path.cwd())

    print("Finished processing files.")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import sys

from pathlib import Path
from subprocess import call


def get_file_list(dir: Path, pattern: str) -> tuple[str, ...]:
    return tuple(str(f) for f in dir.glob(pattern))


def get_files(file_lists: tuple[tuple[str, ...], ...]) -> tuple[str, ...]:
    return tuple(file for file_list in file_lists for file in file_list)


def main():
    patterns = (
        "./include/**/*.h",
        "./tests/**/*.cpp",
        "./tests/**/*.h",
    )

    script_folder = Path(__file__).resolve().parent
    root_dir = script_folder.parent

    file_lists = tuple(get_file_list(root_dir, pattern) for pattern in patterns)
    files = get_files(file_lists)

    clang_format_command = ("clang-format", "-n",
                            "-Werror", "-style=file") + files

    sys.exit(call(clang_format_command))


if __name__ == "__main__":
    main()

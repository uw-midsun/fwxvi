## @file    formatter_utils.py
#  @date    2025-04-20
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Formatter Utils for common operations
#
#  @details
#
#  @ingroup file_formatter

from pathlib import Path
from datetime import datetime
from typing import List, Optional

import argparse
import re


def collapse_blank_lines(lines: List[str], max_blank: int = 1) -> List[str]:
    """
    @brief Collapse multiple empty lines to a minimum amount of blank lines (Default 1)
    @param lines List[str] List of the text/file to modify
    @param max_blank Maximum number of lines that can be empty. Default is 1
    @return Modified text/file with minimized blank lines
    """
    output, blank_count = [], 0
    for ln in lines:
        if ln.strip() == "":
            blank_count += 1
        else:
            blank_count = 0
        if blank_count <= max_blank:
            output.append(ln)
    return output

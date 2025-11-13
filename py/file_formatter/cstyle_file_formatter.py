## @file    cstyle_file_formatter.py
#  @date    2025-04-20
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   C style file formatter for [.c, .h] files
#
#  @details
#
#  @ingroup file_formatter

import re

from pathlib import Path
from datetime import datetime
from typing import List, Optional, Dict
from template_file_formatter import TemplateFileFormatter


class CStyleFileFormatter(TemplateFileFormatter):
    """
    @brief C style file formatter for '.c' and '.h' files
    """

    INCLUDE_CATEGORIES = [
        {"name": "Standard library Headers", "pattern": r'<[a-zA-Z0-9_./]+>'},
        {"name": "Inter-component Headers", "pattern": r'"[^s][a-zA-Z0-9_./]+\.h"'},
        {"name": "Intra-component Headers", "pattern": r''}  # Future expansion? Differentiate the intra/inter component libraries
    ]

    def __init__(self):
        pass

    def format_file(self, file_path: Path):
        """
        @brief Formats a file to Midnight Sun standards
        @param file_path Path Absolute file path to modify
        """
        try:
            lines = file_path.read_text().splitlines(keepends=True)
            data = self._analyze(lines, file_path.name, self._get_file_type(file_path))
            formatted_lines = self._generate(lines, data, file_path.name)
            file_path.write_text("".join(formatted_lines))
            self._log_success(f"[✓] Formatted: {file_path}")
        except FileNotFoundError:
            self._log_error(f"File not found: {file_path}")
        except Exception as e:
            self._log_error(f"Error in {file_path}: {e}")

    def _analyze(self, lines: List[str], file_name: str, file_type: str) -> Dict:
        """
        @brief Analyzes a file for data in a one-pass approach
        @param lines List[str] List containing file content/text to analyze
        @param file_name Name of the file being analyzed
        @param file_type File extension code. For example ".py", ".c" or ".h"
        @return Dictionary containing header file data such as file_name, header_block, etc.
        """
        data = {
            "file_name": file_name,
            "file_type": file_type,
            "header_block": {"lines": None, "info": {}, "start_index": -1, "end_index": -1},
            "pragma_once": {"found": False, "index": -1},
            "includes": {"lines": [], "indices": [], "categories": {cat["name"]: [] for cat in self.INCLUDE_CATEGORIES}},
            "doxygen_groups": {"defgroup_matches": [], "end_count": 0},
            "include_categories_exist": False
        }

        category_headers_count = sum(
            1 for line in lines for cat in self.INCLUDE_CATEGORIES
            if line.strip() == f"/* {cat['name']} */"
        )

        data["include_categories_exist"] = category_headers_count >= len(self.INCLUDE_CATEGORIES)

        header_lines = self._find_header_block(lines)
        if header_lines:
            data["header_block"]["lines"] = header_lines
            data["header_block"]["info"] = self._parse_header_block(header_lines)

        defgroup_pattern = r'@defgroup\s+(\w+)'
        group_end_pattern = r'@\}'

        for i, line in enumerate(lines):
            if header_lines is not None:
                if lines[i:i + len(header_lines)] == header_lines:
                    # If we reached the start of the header and its a direct replica (Surely there isn't two copies of the header in one file)
                    data["header_block"]["start_index"] = i
                    data["header_block"]["end_index"] = i + len(header_lines) - 1

            if line.strip() == "#pragma once":
                data["pragma_once"].update({"found": True, "index": i})

            if not data["include_categories_exist"] and line.strip().startswith("#include"):
                data["includes"]["lines"].append(line)
                data["includes"]["indices"].append(i)
                for category in self.INCLUDE_CATEGORIES:
                    if re.search(category["pattern"], line):
                        data["includes"]["categories"][category["name"]].append(line)
                        break

            if match := re.search(defgroup_pattern, line):
                data["doxygen_groups"]["defgroup_matches"].append((match.group(1), i))
            if re.search(group_end_pattern, line):
                data["doxygen_groups"]["end_count"] += 1

        return data

    def _generate(self, lines: List[str], data: Dict, file_name: str) -> List[str]:
        """
        @brief Generates a formatted file using data from analysis
        @param lines List[str] List containing file content/text to modify
        @param data Dict containing analysis data
        @param file_name Name of the file being analyzed
        @return List[str] of the modified file contents
        """
        output = ["#pragma once\n\n"] if data["file_type"] == ".h" else [""]
        source_info = data["header_block"]["info"]

        if data["header_block"]["lines"] and all(k in source_info for k in ['file', 'brief', 'date', 'author']):
            output.extend(data["header_block"]["lines"])
        else:
            output.extend(self._create_new_header(source_info, file_name))

        output.extend(self._organize_includes_and_content(lines, data))
        return self._collapse_blank_lines(output)

    def _create_new_header(self, source_info: Dict, file_name: str) -> List[str]:
        """
        @brief Creates a new header for the file given a dictionary of header information
        @param source_info Dict containing file name, brief, author, date, etc.
        @param file_name Name of the file being analyzed
        @return List[str] of the new header
        """
        now = datetime.now().strftime("%Y-%m-%d")
        default_brief = Path(file_name).stem.replace('_', ' ').title()
        author = "Midnight Sun Team #24 - MSXVI"
        return [
            "/************************************************************************************************\n",
            f" * @file    {source_info.get('file', file_name).strip()}\n",
            " *\n",
            f" * @brief   {source_info.get('brief', default_brief).strip()}\n",
            " *\n",
            f" * @date    {now}\n",
            f" * @author  {author}\n",
            " ************************************************************************************************/\n",
            ""
        ]

    def _organize_includes_and_content(self, lines: List[str], data: Dict) -> List[str]:
        """
        @brief Organizes file includes and the remaining content
        @param lines List[str] List containing file content/text to organize
        @param data Dict containing analysis information
        @return List[str] of the reorganized file
        """
        formatted = []
        if not data["include_categories_exist"]:
            for cat in self.INCLUDE_CATEGORIES:
                formatted.append(f"\n/* {cat['name']} */\n")
                formatted.extend(sorted(data["includes"]["categories"].get(cat["name"], [])))

            if data["file_type"] == ".h":
                # H files
                content_start = max(
                    data["header_block"]["end_index"] + 1 if data["header_block"]["end_index"] >= 0 else 0,
                    data["pragma_once"]["index"] + 1 if data["pragma_once"]["index"] >= 0 else 0
                )
            else:
                # C files
                content_start = data["header_block"]["end_index"] + 1 if data["header_block"]["end_index"] >= 0 else 0

            filtered = [
                line for i, line in enumerate(lines)
                if i >= content_start
                and i not in data["includes"]["indices"]
                and not any(h in line.strip() for h in [cat["name"] for cat in self.INCLUDE_CATEGORIES])
            ]

            if data["file_type"] == ".h":
                # H files
                return formatted + self._handle_doxygen_group(filtered, data)

            # C files
            formatted.extend(filtered)
            return formatted

        # Else if the include categories do exist, get the line number of the end of the include section
        # Then copy the remaining content beyond this point
        last_include_end = self._find_last_include_boundary(lines, data)

        if data["file_type"] == ".h":
            # H files
            base_start = max(data["header_block"]["end_index"], data["pragma_once"]["index"]) + 1
            formatted.extend(lines[base_start:last_include_end])
            remaining = lines[last_include_end:]
            return formatted + self._handle_doxygen_group(remaining, data)

        # C files
        base_start = data["header_block"]["end_index"] + 1
        formatted.extend(lines[base_start:last_include_end])
        remaining = lines[last_include_end:]
        formatted.extend(remaining)
        return formatted

    def _handle_doxygen_group(self, content: List[str], data: Dict) -> List[str]:
        """
        @brief Handles doxygen groups that wrap the file comments
        @param content List[str] List containing the remaining file content/text to append
        @param data Dict containing analysis information
        @return List[str] of the file with valid doxygen groups
        """
        formatted = []
        if not data["doxygen_groups"]["defgroup_matches"] and data["file_name"].endswith(".h"):
            component = data["file_name"].split(".")[0]
            formatted.extend([
                "\n",
                "/**\n",
                f" * @defgroup {component}\n",
                f" * @brief    {component} Firmware\n",
                " * @{\n",
                " */\n",
                "\n"
            ])
            formatted.extend(content)

            if data["doxygen_groups"]["end_count"] != 1:
                # Add the closing doxygen line
                formatted.append("\n/** @} */\n")
        else:
            formatted.extend(content)
            if (data["doxygen_groups"]["defgroup_matches"] and len(data["doxygen_groups"]["defgroup_matches"]) > data["doxygen_groups"]["end_count"]):
                # Add the closing doxygen line
                formatted.append("\n/** @} */\n")
        return formatted

    def _find_last_include_boundary(self, lines: List[str], data: Dict) -> int:
        """
        @brief Searches for the end of the include section, for updating the insertion index of remaining content
        @param lines List[str] List containing the file content/text to search
        @param data Dict containing analysis information
        @return int line number for the end of the include section
        """
        if data["file_type"] == ".h":
            # H files
            last_include_index = max(
                data["header_block"]["end_index"],
                data["pragma_once"]["index"]
            ) + 1
        else:
            # C files
            last_include_index = data["header_block"]["end_index"] + 1

        in_include_section = False
        category_count = 0
        for i, line in enumerate(lines[last_include_index:], start=last_include_index):
            if line.strip().startswith("/* ") and any(cat["name"] in line for cat in self.INCLUDE_CATEGORIES):
                category_count += 1
                in_include_section = True

            if in_include_section and (line.strip().startswith("/**") or
                                       (line.strip() == "" and i + 1 < len(lines) and
                                        not lines[i + 1].strip().startswith("#include"))):
                in_include_section = False
                if category_count >= len(self.INCLUDE_CATEGORIES):
                    return i
        return len(lines)

    def _find_header_block(self, lines: List[str]) -> Optional[List[str]]:
        """
        @brief Finds the header block within the file
        @param lines List[str] List containing the file content/text to search
        @return Optional List[str] of the header lines. If nothing is found it will return None
        """
        header_lines = []
        in_header = False
        start_regex = r'/\*+.*'
        end_regex = r'\*+/'

        for line in lines:
            stripped = line.strip()
            if not in_header and re.match(start_regex, stripped):
                header_lines.append(line)
                in_header = True
            elif in_header:
                header_lines.append(line)
                if re.fullmatch(end_regex, stripped):
                    # Check if this block contains @file
                    if any('@file' in l for l in header_lines):
                        return header_lines
                    # This was just a section banner — ignore it
                    header_lines = []
                    in_header = False
        return None

    def _parse_header_block(self, header_lines: List[str]) -> Dict:
        """
        @brief Parse the header block for file, brief, date, and author information
        @param header_lines List[str] List containing the header lines to parses
        @return Dict containing file header information
        """
        info = {}
        patterns = {
            "file": r'@file\s+(.+)',
            "brief": r'@brief\s+(.+)',
            "date": r'@date\s+(.+)',
            "author": r'@author\s+(.+)'
        }
        for line in header_lines:
            for key, pattern in patterns.items():
                if match := re.search(pattern, line):
                    info[key] = match.group(1).strip()
        return info

## @file    template_file_formatter.py
#  @date    2025-04-20
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Template for file formatter
#
#  @details
#
#  @ingroup file_formatter

from abc import ABC, abstractmethod
from pathlib import Path
from typing import List, Dict


class TemplateFileFormatter(ABC):
    """
    @brief Abstract template class
    """

    @abstractmethod
    def format_file(self, file_path: Path):
        """
        @brief Format the file
        """
        pass  # pylint: disable=unnecessary-pass

    def format_files(self, file_paths: List[Path]):
        """
        @brief Formats multiple files to Midnight Sun standards
        @param file_paths List[Path] List of absolute file path to modify
        """
        for path in file_paths:
            self.format_file(path)

    @abstractmethod
    def _analyze(self, lines: List[str], file_name: str, file_type: str) -> Dict:
        """
        @brief Analyze the contents of a file to collect structural info
        """
        pass  # pylint: disable=unnecessary-pass

    @abstractmethod
    def _generate(self, lines: List[str], data: Dict, file_name: str) -> List[str]:
        """
        @brief Generate the new content of the file based on analysis
        """
        pass  # pylint: disable=unnecessary-pass

    def _collapse_blank_lines(self, lines: List[str], max_blank: int = 1) -> List[str]:
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

    def _get_file_type(self, file_path: Path) -> str:
        """
        @brief Analyze the contents of a file to collect structural info
        @param file_path Path File to analyze for its type
        """
        return file_path.suffix.lower()

    def _log_success(self, message: str):
        print(f"[SUCCESS] {message}")

    def _log_info(self, message: str):
        print(f"[INFO] {message}")

    def _log_warning(self, message: str):
        print(f"[WARNING] {message}")

    def _log_error(self, message: str):
        print(f"[ERROR] {message}")

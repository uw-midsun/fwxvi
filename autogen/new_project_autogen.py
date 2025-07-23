## @file    new_project_autogen.py
#  @date    2024-12-21
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Updates the required data for new project autogeneration
#
#  @details This module provides functionality to autogenerate template selection, config.json, and new project naming
#
#  @ingroup autogen

from pathlib    import Path
from datetime   import datetime

def get_data(args):
    """
    @brief Handles new project autogeneration
    
    @param args Arguments object containing output path and template information
    
    @return Dictionary containing the project name and the included libraries
    """
    project_name = Path(args.output).stem
    project_type = Path(args.output).parent.stem

    libs = []
    if project_type == "projects":
        libs = ["FreeRTOS", "ms-common", "master"]
        args.template_name = "new_project"
    
    if project_type == "smoke":
        libs = ["FreeRTOS", "ms-common"]
        args.template_name = "new_smoke"
    
    if project_type == "py":
        args.template_name = "new_py"
    
    if Path(args.output).exists():
        raise (f"{project_type}/{project_name} already exists")
    
    current_date = datetime.now()
    current_date = current_date.strftime("%Y-%m-%d")

    return {"project_name": project_name, "libs": libs, "current_date" : current_date }

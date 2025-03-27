## @defgroup autogen Autogeneration tools
#  This package contains parsing and generation utilities
#
## @file    __main__.py
#  @date    2024-12-21
#  @author  Midnight Sun Team #24 - MSXVI
#  @brief   Main interface for autogeneration for new files, CAN and tests
#
#  @ingroup autogen

import argparse
import jinja2
from pathlib import Path

def main():
    """
    @brief Processes the Jinja template and destination and creates autogenerated file
    """
    parser = argparse.ArgumentParser()
    parser.add_argument("template_name")
    parser.add_argument("-o", dest="output", required=True, metavar="DIR")

    args = parser.parse_args()

    if args.template_name in ["cantools", "system_can", "project_can", "simulation_app"]:
        from .can_autogen import get_data
    elif args.template_name in ["test"]:
        from .test_autogen import get_data
    elif args.template_name in ["new_project", "new_py"]:
        from .new_project_autogen import get_data

    data = get_data(args)

    template_dir = Path("autogen/templates", args.template_name)
    template_loader = jinja2.FileSystemLoader(searchpath=template_dir)
    env = jinja2.Environment(loader=template_loader)
    env.tests["contains"] = (lambda list, var: (var in list))

    for template in template_dir.glob("**/*.jinja"):
        template_path = str(template.relative_to(template_dir))

        # Create the file text by rendering it with provided data (Must be dictionary)
        output = env.get_template(template_path).render(**data)

        # Create the output file name with provided data
        # For example: template is called {{board}}_rx.c.jinja -> bms_carrier_rx.c.jinja
        output_file_name = env.from_string(template_path).render(**data)

        # Create output file without the .jinja extension
        output_file = Path(args.output, output_file_name).with_suffix("")
        
        # Create parent directories if it does not exist
        output_file.parent.mkdir(parents=True, exist_ok=True)

        # Write output text to the file
        output_file.write_text(output)

    print("Done autogenerating")

if __name__ == "__main__":
    main()

<!--
General guidelines
These are just guidelines, not strict rules - document however seems best.
A README for a firmware-only project (e.g. Babydriver, MPXE, bootloader, CAN explorer) should answer the following questions:
    - What is it?
    - What problem does it solve?
    - How do I use it? (with usage examples / example commands, etc)
    - How does it work? (architectural overview)
A README for a board project (powering a hardware board, e.g. power distribution, centre console, charger, BMS carrier) should answer the following questions:
    - What is the purpose of the board?
    - What are all the things that the firmware needs to do?
    - How does it fit into the overall system?
    - How does it work? (architectural overview, e.g. what each module's purpose is or how data flows through the firmware)
-->
# platform

DEFAULT:
    Meant for flashing MS16s memory map:

        Bootstrap
        Bootloader
        Active application
        Staging application
        FS storage

LEGACY:
    Only meant for flashing the application. This is what was used on previous versions of the car.

- hardware/
    - templates/
        - default/
            - Contains linkerscript template for flashing the following:
                - Bootstrap
                - Bootloader
                - Active application
                - Staging application
                - FS storage
        - legacy/
            - Contains linkerscript template for legacy flashing (Only the application)

    - STM32L4P5VET6
        - default
            - memory_map.ld
            - device_flash_params.cfg
        - legacy
            - memory_map.ld
            - device_flash_params.cfg
    - STM32L433CCU6
        - default
            - memory_map.ld
            - device_flash_params.cfg
        - legacy
            - memory_map.ld
            - device_flash_params.cfg
    - STM32L496RGT6
        - default
            - memory_map.ld
            - device_flash_params.cfg
        - legacy
            - memory_map.ld
            - device_flash_params.cfg
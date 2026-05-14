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
# lvgl

Holds firmware's lvgl9 source code, and is submoduled. Used by our steering GUI. You may see more about our implementation in libraries/ms-gui/. This is a copy the latest version as of March 15, 2026, here is the exact [commit](https://github.com/lvgl/lvgl/commit/1e334db0d2c839aa0de56e35c0fcf7abc87faa0b). Note: I did not submodule the actual repository or a forked version as we do not need all the extra jargon (examples, tests, etc.),

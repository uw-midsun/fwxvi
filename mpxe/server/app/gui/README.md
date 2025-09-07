# MPXE GUI NOTES

## How to Build MPXE GUI 
Note: This only works for Linux

### Quick Checks to make sure you have Qt6 
Ensure you have QT6, check by running
```bash
qmake6 --version
```
If you don't see an output like `QMake version 3.1` then you don't have it installed, install it using:
```bash
sudo apt install qt6-base-dev
```
Ensure it is in the correct directory, by running:
```bash
which qmake6
```
You should see the following output:
```
/usr/bin/qmake6
```
If it is not there, please move it there 

### Building it
To Build:
```bash
scons mpxe-gui --platform=x86
```
Should automatically launch the GUI application, if it hasn't, or if you just want to run the GUI app again, you can just run the executable directly:
```bash
./build/x86/bin/projects/mpxe_gui 
```



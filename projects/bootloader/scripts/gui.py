import tkinter as tk
from tkinter import ttk, filedialog
import sv_ttk
from ctypes import windll

from log_manager import update_log
from flash_application import Flash_Application
from jump_application import Jump_Application
from jump_bootloader import Jump_Bootloader

windll.shcore.SetProcessDpiAwareness(1)

root = tk.Tk()
root.title("Midnight Sun Bootloader")

style = ttk.Style()
style.configure("Custom.TButton", font=("Helvetica", 20),padding=(10, 5)) 

# Configure rows in the root window to stretch
root.grid_rowconfigure(0, weight=1)
root.grid_rowconfigure(1, weight=1)
root.grid_rowconfigure(2, weight=1)
root.grid_rowconfigure(3, weight=1)

# Create a canvas for the scrollable area, spanning rows 0, 1, 2, and 3
canvas = tk.Canvas(root, height=500, width=500)
canvas.grid(column=2, row=0, rowspan=4, padx=20, pady=20, sticky='nsew')  # Ensure canvas spans 4 rows and stretches

# Create a scrollbar and link it to the canvas
scrollbar = ttk.Scrollbar(root, orient="vertical", command=canvas.yview)
scrollbar.grid(column=3, row=0, rowspan=4, sticky='ns')  # Position scrollbar

canvas.config(yscrollcommand=scrollbar.set)

# Create a LabelFrame to hold the labels inside the canvas
log_frame = ttk.LabelFrame(canvas, text="Command Progress", padding=(20, 10), height=500)
log_frame.grid(column=0, row=0, padx=10, pady=10)

# Add the log_frame to the canvas using create_window
canvas.create_window((0, 0), window=log_frame, anchor="nw")

# Create a label in the log_frame to display logs or progress
log_label = ttk.Label(log_frame, text="Waiting for command...", width=50)
log_label.grid(column=1, row=0, padx=10, pady=10)

# Update the scrollregion to match the size of the canvas content
log_frame.update_idletasks()
canvas.config(scrollregion=canvas.bbox("all"))



alignment_var = tk.StringVar()
alignments = ('Flash', 'App', 'Bootloader')


lf = ttk.LabelFrame(root, text='Command', padding=20)
lf.grid(column=0, row=0, padx=20, pady=20)

grid_column = 0
for alignment in alignments:
    radio = ttk.Radiobutton(lf, text=alignment, value=alignment, variable=alignment_var, command=lambda: update_ui())
    radio.grid(column=grid_column, row=0, sticky='w', ipadx=10)  # Align to the left
    grid_column += 1

alignment_var.set('Flash')


# Create a LabelFrame for node IDs and add Spinboxes (Spinner boxes) for node ID inputs
node_frame = ttk.LabelFrame(root, text="Node IDs", padding=20)
node_frame.grid(column=1, row=0, padx=20, pady=20, rowspan=4)  # This spans rows 0 to 3 in column 1

# Label for Node IDs
node_label = ttk.Label(node_frame, text="Enter Node IDs:", font=("Helvetica", 12))
node_label.grid(column=0, row=0, padx=10, pady=10)

# Initialize the list to keep track of added Spinboxes
spinboxes = []

# Function to add a new Spinbox for Node ID input
def add_spinbox():
    row = len(spinboxes) + 1  # Dynamically determine the row for the new Spinbox
    spinbox = ttk.Spinbox(node_frame, from_=1, to=100, width=5)
    spinbox.grid(column=0, row=row, padx=10, pady=5)
    spinboxes.append(spinbox)

    update_ui()

def update_ui():
    selection = alignment_var.get()

    # reset the UI
    selected_file_label.grid_forget()
    open_file_button.grid_forget()
    flash_button.grid_forget()
    jump_app_button.grid_forget()
    bootloader_button.grid_forget()

    plus_button.grid(column=0, row=len(spinboxes) + 2, padx=10, pady=10)

    if selection == "Flash":
        flash_button.grid(column=0, row=1, padx=20, pady=10, sticky='n')
        selected_file_label.grid(column=0, row=2, padx=20, pady=10, sticky='s')
        open_file_button.grid(column=0, row=3, padx=20, pady=10, sticky='n')
    elif selection == "App":
        jump_app_button.grid(column=0, row=1, padx=20, pady=10, sticky='n')
    elif selection == "Bootloader":
        bootloader_button.grid(column=0, row=1, padx=20, pady=10, sticky='n')


file_path = ""
def open_file():
    global file_path
    file_path = filedialog.askopenfilename(
        title="Select a .bin file",
        filetypes=[("Binary Files", "*.bin"), ("All Files", "*.*")]
    )

    if file_path:
        selected_file_label.config(text=f"Selected File:\n{file_path}")
    
def get_node_ids():
    # Get node IDs from all spinboxes
    node_ids = []  # Initialize an empty list to store valid node IDs
    for spinbox in spinboxes:
        value = spinbox.get()  # Get the value from the Spinbox
        if value.isdigit():  # Check if the value consists only of digits (valid integer)
            node_ids.append(int(value))  # Convert to integer and add to the list


    return node_ids

def flash_app():
    node_ids = get_node_ids()

    if not node_ids:
        update_log("No valid Node IDs entered.", error=True)
        return
    
    if file_path == "":
        update_log("No file selected", error=True)
        return
    
    update_log(f"Starting Flash Process for nodes: {node_ids}")
    
    try:
        flash_app = Flash_Application(file_path)
        flash_app.start_flash(node_ids=node_ids)
        update_log(f"Firmware flashed successfully on nodes {node_ids}")
    except Exception as e:
        update_log(f"Error during flashing: {str(e)}")


def jump_app():
    node_ids = get_node_ids()
    jump_app = Jump_Application()

    if not node_ids:
        update_log("No valid Node IDs entered.", error=True)
        return
    
    update_log("Starting Jump Process...")

    try:
        jump_app.jump_application(node_ids=node_ids)
    except Exception as e:
        update_log(f"Error during jump: {str(e)}")


def bootloader_app():
    node_ids = get_node_ids()
    jump_bootloader = Jump_Bootloader()

    if not node_ids:
        update_log("No valid Node IDs entered.", error=True)
        return
    
    update_log("Starting Bootloader Process...")

    try:
        jump_bootloader.jump_bootloader(node_ids=node_ids)
    except Exception as e:
        update_log(f"Error during jump: {str(e)}")




selected_file_label = ttk.Label(root, text="No file selected")
open_file_button = ttk.Button(root, text="Open File", command=open_file, style="TButton", width=20)
flash_button = ttk.Button(root, text="FLASH START", command=flash_app, style="TButton", width=30)

bootloader_button = ttk.Button(root, text="BOOTLOADER START", command=bootloader_app, style="TButton", width=30)

jump_app_button = ttk.Button(root, text="JUMP APP START", command=jump_app, style="Custom.TButton", width=30)

plus_button = ttk.Button(node_frame, text="+", command=add_spinbox, width=5)


sv_ttk.set_theme("dark")

update_ui()
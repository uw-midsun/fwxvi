import tkinter as tk
from tkinter import ttk, filedialog
import sv_ttk
from ctypes import windll

windll.shcore.SetProcessDpiAwareness(1)
root = tk.Tk()
root.title("Midnight Sun Bootloader")


lf = ttk.LabelFrame(root, text='Command')
lf.grid(column=0, row=0, padx=20, pady=20)

alignment_var = tk.StringVar()
alignments = ('Flash', 'App', 'Bootloader')


grid_column = 0
for alignment in alignments:
    radio = ttk.Radiobutton(lf, text=alignment, value=alignment, variable=alignment_var)
    radio.grid(column=grid_column, row=0, ipadx=10, ipady=10)
    grid_column += 1

def open_file():
    file_path = filedialog.askopenfilename(
        title="Select a .bin file",
        filetypes=[("Binary Files", "*.bin"), ("All Files", "*.*")]
    )
    if file_path:
        selected_file_label.config(text=f"Selected File: {file_path}")

# Add a button to open the file dialog
open_file_button = ttk.Button(root, text="Open File", command=open_file)
open_file_button.grid(column=0, row=1, padx=20, pady=10)

# Add a label to display the selected file's path
selected_file_label = ttk.Label(root, text="No file selected")
selected_file_label.grid(column=0, row=2, padx=20, pady=10)

sv_ttk.set_theme("dark")
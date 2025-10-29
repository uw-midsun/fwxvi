import cantools
import struct
import time

import tkinter as tk
import can
from collections import deque, defaultdict
from collections import deque
import threading

cache=defaultdict(lambda:deque(maxlen=50))
cache_lock=threading.Lock()
gui_labels={}

try:
    db=cantools.database.load_file('system_dbc.dbc')
except FileNotFoundError:
    print(f"DBC file not found")
    exit()

try:
    bus = can.interface.Bus(channel='virtual', bustype='virtual')
except OSError:
    print("Virtual CAN bus not setup correctly")
    exit()
try:
    bus=can.interface.Bus(channel='can0', bustype='socketcan')
except OSError:
    print(f"Can BUS not found")
cache={}


def listener_thread():
    while(1):
        msg=bus.recv()
        if msg is None:
            continue
        decoded_data=decoder(msg)

        if decoded_data:
            with cache_lock:
                cache[msg.arbitration_id].append(decoded_data)
 
def decoder(msg):
    try:
        decoded_data=db.decode_message(msg.arbitration_id, msg.data)
        return decoded_data
    except(KeyError, ValueError):
        return None  
       
def run_gui():
    root=tk.Tk()
    root.title("CAN Explorer")
    root.geometry("500x700")

if __name__ == "__main__":
    run_gui()




    


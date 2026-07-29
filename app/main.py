"""
DS18B20 Temperature Monitor — PC Application
Entry point: launches the tkinter GUI and manages serial communication.

Usage:
    python main.py

Requirements:
    pip install pyserial
"""

import tkinter as tk
import queue
import sys

from serial_handler import SerialHandler
from ui import TemperatureMonitorApp


def main():
    # Create shared queue for thread-safe communication
    data_queue = queue.Queue()

    # Create serial handler
    handler = SerialHandler(data_queue)

    # Create tkinter root
    root = tk.Tk()

    # Build the app UI
    app = TemperatureMonitorApp(root, handler)

    # Handle window close properly
    root.protocol("WM_DELETE_WINDOW", app.on_close)

    # Start the GUI main loop
    root.mainloop()


if __name__ == "__main__":
    main()

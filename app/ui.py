"""
Temperature Monitor UI — tkinter-based GUI.
Displays real-time temperature from DS18B20 sensor via serial port.
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
import queue
import time


class TemperatureMonitorApp:
    """Main application window."""

    # Temperature color thresholds (°C)
    COLOR_COLD = "#3498db"    # Blue: below 10°C
    COLOR_NORMAL = "#27ae60"  # Green: 10-35°C
    COLOR_HOT = "#e74c3c"     # Red: above 35°C

    def __init__(self, root: tk.Tk, serial_handler):
        self.root = root
        self.handler = serial_handler
        self.temp_history: list[tuple[str, float]] = []  # (timestamp, temp)

        self._build_ui()
        self._start_polling()

    def _build_ui(self):
        """Construct the GUI layout."""
        self.root.title("DS18B20 Temperature Monitor")
        self.root.geometry("480x460")
        self.root.resizable(False, False)
        self.root.configure(bg="#f0f0f0")

        # ── Connection Frame ──
        conn_frame = ttk.LabelFrame(self.root, text="Serial Connection", padding=8)
        conn_frame.pack(fill="x", padx=10, pady=(10, 5))

        # Row 0: COM port
        ttk.Label(conn_frame, text="Port:").grid(row=0, column=0, sticky="w", padx=(0, 4))
        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(conn_frame, textvariable=self.port_var, width=14, state="readonly")
        self.port_combo.grid(row=0, column=1, sticky="w")
        self._refresh_ports()
        ttk.Button(conn_frame, text="↻", width=3, command=self._refresh_ports).grid(row=0, column=2, padx=2)

        # Row 0: Baud rate
        ttk.Label(conn_frame, text="Baud:").grid(row=0, column=3, sticky="w", padx=(10, 4))
        self.baud_var = tk.StringVar(value="115200")
        baud_combo = ttk.Combobox(
            conn_frame, textvariable=self.baud_var, width=8,
            values=["9600", "19200", "38400", "57600", "115200"], state="readonly"
        )
        baud_combo.grid(row=0, column=4, sticky="w")

        # Row 1: Connect / Disconnect
        btn_frame = ttk.Frame(conn_frame)
        btn_frame.grid(row=1, column=0, columnspan=5, pady=(8, 0))
        self.connect_btn = ttk.Button(btn_frame, text="Connect", command=self._toggle_connection)
        self.connect_btn.pack(side="left", padx=2)
        ttk.Button(btn_frame, text="Request Once", command=self._request_once).pack(side="left", padx=2)

        # ── Temperature Display Frame ──
        temp_frame = ttk.LabelFrame(self.root, text="Current Temperature", padding=12)
        temp_frame.pack(fill="x", padx=10, pady=5)

        self.temp_label = tk.Label(
            temp_frame, text="--.- °C",
            font=("Segoe UI", 48, "bold"), fg="#888", bg="#f0f0f0"
        )
        self.temp_label.pack()

        self.update_label = tk.Label(
            temp_frame, text="Not connected",
            font=("Segoe UI", 9), fg="#888", bg="#f0f0f0"
        )
        self.update_label.pack()

        # ── Data Log Frame ──
        log_frame = ttk.LabelFrame(self.root, text="Data Log", padding=4)
        log_frame.pack(fill="both", expand=True, padx=10, pady=5)

        self.log_text = scrolledtext.ScrolledText(
            log_frame, height=10, font=("Consolas", 9), wrap=tk.WORD,
            state="disabled"
        )
        self.log_text.pack(fill="both", expand=True)

        # ── Status Bar ──
        status_frame = ttk.Frame(self.root)
        status_frame.pack(fill="x", padx=10, pady=(0, 8))
        self.status_label = tk.Label(
            status_frame, text="Disconnected",
            font=("Segoe UI", 9), fg="#888", anchor="w"
        )
        self.status_label.pack(fill="x")
        ttk.Separator(status_frame, orient="horizontal").pack(fill="x", pady=(2, 0))

    def _refresh_ports(self):
        """Refresh the COM port dropdown list."""
        ports = self.handler.list_ports()
        self.port_combo["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def _toggle_connection(self):
        """Connect or disconnect from the serial port."""
        if self.handler.is_connected():
            self.handler.disconnect()
            self._update_connection_ui(False)
        else:
            port = self.port_var.get()
            if not port:
                messagebox.showwarning("No Port", "Please select a COM port.")
                return
            baude = int(self.baud_var.get())
            if self.handler.connect(port, baude):
                self._update_connection_ui(True)
            else:
                messagebox.showerror(
                    "Connection Failed",
                    f"Could not open {port}.\nCheck that the device is plugged in."
                )

    def _update_connection_ui(self, connected: bool):
        """Update UI elements based on connection state."""
        if connected:
            self.connect_btn.configure(text="Disconnect")
            self.status_label.configure(text=f"Connected → {self.handler.port_name} @ {self.handler.baudrate} bps", fg="#27ae60")
            self.update_label.configure(text="Waiting for data...", fg="#888")
        else:
            self.connect_btn.configure(text="Connect")
            self.status_label.configure(text="Disconnected", fg="#888")
            self.temp_label.configure(text="--.- °C", fg="#888")
            self.update_label.configure(text="Not connected", fg="#888")

    def _request_once(self):
        """Manually request a temperature reading."""
        if self.handler.is_connected():
            self.handler.send_request()
        else:
            messagebox.showinfo("Not Connected", "Connect to a COM port first.")

    def _start_polling(self):
        """Start polling the data queue from the serial thread."""
        self._poll_queue()

    def _poll_queue(self):
        """Check for incoming data from the serial handler."""
        try:
            while True:
                msg_type, data = self.handler.data_queue.get_nowait()
                if msg_type == "temperature":
                    self._update_temperature(data)
                elif msg_type == "raw":
                    self._append_log(data)
                elif msg_type == "error":
                    self._append_log(f"[ERROR] {data}")
                elif msg_type == "status":
                    pass  # Handled in _toggle_connection
        except queue.Empty:
            pass

        # Schedule next poll
        self.root.after(100, self._poll_queue)

    def _update_temperature(self, temp: float):
        """Update the temperature display with color coding."""
        # Determine color based on temperature
        if temp < 10.0:
            color = self.COLOR_COLD
        elif temp > 35.0:
            color = self.COLOR_HOT
        else:
            color = self.COLOR_NORMAL

        self.temp_label.configure(text=f"{temp:.1f} °C", fg=color)
        self.update_label.configure(
            text=f"Last update: {time.strftime('%H:%M:%S')}",
            fg="#666"
        )

        # Keep history
        self.temp_history.append((time.strftime("%H:%M:%S"), temp))
        if len(self.temp_history) > 50:
            self.temp_history.pop(0)

    def _append_log(self, text: str):
        """Append a timestamped line to the log view."""
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.configure(state="normal")
        self.log_text.insert(tk.END, f"[{timestamp}] {text}\n")
        self.log_text.see(tk.END)  # Auto-scroll
        self.log_text.configure(state="disabled")

    def on_close(self):
        """Clean up on window close."""
        if self.handler.is_connected():
            self.handler.disconnect()
        self.root.destroy()

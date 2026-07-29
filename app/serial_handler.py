"""
Serial port handler for DS18B20 temperature monitor.
Runs in a background thread, reads lines from the COM port,
parses temperature values, and sends them to the UI via a queue.
"""

import threading
import queue
import re

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("Error: pyserial not installed. Run: pip install pyserial")
    raise


class SerialHandler:
    """Manages serial port connection and data parsing."""

    # Regex to match temperature data: "T:25.3" or "T:-10.5"
    TEMP_PATTERN = re.compile(r"T:\s*(-?\d+\.?\d*)")

    def __init__(self, data_queue: queue.Queue):
        self.data_queue = data_queue
        self.ser: serial.Serial | None = None
        self.thread: threading.Thread | None = None
        self.running = False
        self.port_name = ""
        self.baudrate = 115200

    @staticmethod
    def list_ports() -> list[str]:
        """Return list of available COM port names."""
        ports = serial.tools.list_ports.comports()
        return [p.device for p in sorted(ports)]

    @staticmethod
    def list_ports_with_desc() -> list[tuple[str, str]]:
        """Return list of (port_name, description) tuples."""
        ports = serial.tools.list_ports.comports()
        return [(p.device, p.description) for p in sorted(ports)]

    def connect(self, port: str, baudrate: int = 115200) -> bool:
        """Open serial port and start reader thread."""
        try:
            self.ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=1.0,
            )
            self.port_name = port
            self.baudrate = baudrate
            self.running = True

            # Flush any stale data
            self.ser.reset_input_buffer()

            # Start reader thread
            self.thread = threading.Thread(target=self._reader_loop, daemon=True)
            self.thread.start()

            # Send a request immediately
            self.send_request()

            return True
        except serial.SerialException as e:
            self.data_queue.put(("error", str(e)))
            return False

    def disconnect(self):
        """Close serial port and stop reader thread."""
        self.running = False
        if self.thread and self.thread.is_alive():
            self.thread.join(timeout=2.0)
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except serial.SerialException:
                pass
        self.ser = None
        self.data_queue.put(("status", "Disconnected"))

    def send_request(self):
        """Send 'R' command to request temperature from device."""
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(b"R\n")
            except serial.SerialException:
                self.data_queue.put(("error", "Serial write failed"))

    def is_connected(self) -> bool:
        return self.ser is not None and self.ser.is_open and self.running

    def _reader_loop(self):
        """Background thread: read lines from serial port."""
        line_buffer = ""
        while self.running:
            try:
                if self.ser and self.ser.is_open and self.ser.in_waiting > 0:
                    chunk = self.ser.read(self.ser.in_waiting).decode("utf-8", errors="replace")
                    line_buffer += chunk

                    # Process complete lines
                    while "\n" in line_buffer:
                        line, line_buffer = line_buffer.split("\n", 1)
                        self._parse_line(line.strip())
                else:
                    # Small sleep to avoid busy-waiting
                    threading.Event().wait(0.05)
            except serial.SerialException:
                if self.running:
                    self.data_queue.put(("error", "Serial connection lost"))
                break
            except Exception as e:
                if self.running:
                    self.data_queue.put(("error", str(e)))

    def _parse_line(self, line: str):
        """Parse a received line and extract temperature."""
        if not line:
            return

        # Check for error messages from device
        if line.startswith("ERR"):
            self.data_queue.put(("error", line))
            return

        # Try to find temperature in the line
        match = self.TEMP_PATTERN.search(line)
        if match:
            try:
                temp = float(match.group(1))
                self.data_queue.put(("temperature", temp))
            except ValueError:
                pass

        # Forward raw line for log display
        self.data_queue.put(("raw", line))

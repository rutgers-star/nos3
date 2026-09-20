#!/usr/bin/env python3
"""
SBN Client for Raspberry Pi
Implements SBN protocol to communicate with NOS3/cFS

This runs on the Raspberry Pi and connects to the SBN network
"""

import socket
import struct
import time
import threading
import logging
from typing import Optional, Tuple, Dict, Any
import json

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class SBNClient:
    """SBN Client implementation for Raspberry Pi"""

    # SBN Message Types
    SBN_NO_MSG = 0x00
    SBN_SUB_MSG = 0x01
    SBN_UNSUB_MSG = 0x02
    SBN_APP_MSG = 0x03
    SBN_PROTO_MSG = 0x04

    # Custom Raspberry Pi message types
    SBN_SENSOR_MSG = 0x80
    SBN_CMD_MSG = 0x81
    SBN_STATUS_MSG = 0x82
    SBN_FILE_MSG = 0x83

    # Configuration (must match SBN configuration)
    PROCESSOR_ID = 100  # Unique ID for this Raspberry Pi
    SPACECRAFT_ID = 0
    PROTOCOL_VERSION = 1  # Current SBN protocol version (from doc)

    # Message IDs (must match raspberry_pi_msgids.h)
    RASPBERRY_PI_DATA_IN_MID = 0x0902
    RASPBERRY_PI_DATA_OUT_MID = 0x1902

    def __init__(self, host: str, port: int = 2234):  # Match SBN_CLIENT_PORT
        """
        Initialize SBN Client

        Args:
            host: NOS3 host address
            port: SBN port (default 2235)
        """
        self.host = host
        self.port = port
        self.socket: Optional[socket.socket] = None
        self.connected = False
        self.running = False
        self.subscriptions = set()
        self.receive_thread: Optional[threading.Thread] = None
        self.handlers = {}

    def connect(self) -> bool:
        """Connect to SBN server"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.host, self.port))
            self.connected = True
            logger.info(f"Connected to SBN at {self.host}:{self.port}")

            # Send protocol version message
            self._send_protocol_msg()

            # Start receive thread
            self.running = True
            self.receive_thread = threading.Thread(target=self._receive_loop)
            self.receive_thread.start()

            # Subscribe to commands from FSW
            self.subscribe(self.RASPBERRY_PI_DATA_OUT_MID)

            return True

        except Exception as e:
            logger.error(f"Failed to connect: {e}")
            return False

    def disconnect(self):
        """Disconnect from SBN server"""
        self.running = False
        if self.socket:
            self.socket.close()
        if self.receive_thread:
            self.receive_thread.join()
        self.connected = False
        logger.info("Disconnected from SBN")

    def _pack_sbn_header(self, msg_type: int, payload_size: int) -> bytes:
        """
        Pack SBN header (big-endian)

        Format (from SBN doc):
            uint16 MsgSz - Size excluding header
            uint8  MsgType
            uint32 ProcessorID (CpuID)
            uint32 SpacecraftID (if protocol >= version 6)
        """
        # Current NOS3 SBN version doesn't include SpacecraftID in header
        return struct.pack('>HBI',
                          payload_size,  # Message size (excluding header)
                          msg_type,      # Message type
                          self.PROCESSOR_ID)  # Processor ID

    def _send_protocol_msg(self):
        """Send protocol version message"""
        payload = struct.pack('B', self.PROTOCOL_VERSION)
        header = self._pack_sbn_header(self.SBN_PROTO_MSG, len(payload))
        self.socket.send(header + payload)
        logger.debug(f"Sent protocol version: {self.PROTOCOL_VERSION}")

    def subscribe(self, msg_id: int):
        """Subscribe to a message ID"""
        if msg_id not in self.subscriptions:
            self.subscriptions.add(msg_id)
            # Send subscription message
            payload = struct.pack('>HH', 1, msg_id)  # Count=1, MsgId
            header = self._pack_sbn_header(self.SBN_SUB_MSG, len(payload))
            self.socket.send(header + payload)
            logger.info(f"Subscribed to MID 0x{msg_id:04X}")

    def send_sensor_data(self, sensor_data: Dict[str, Any]):
        """Send sensor data to NOS3 as application message with proper CCSDS formatting"""
        # Build CCSDS primary header (compatible with reference implementation)
        # StreamId: RASPBERRY_PI_DATA_IN_MID with version/type bits
        stream_id = self.RASPBERRY_PI_DATA_IN_MID | 0x0800  # Secondary header present
        sequence = 0xC000 | (self._get_next_sequence() & 0x3FFF)  # Standalone packet + sequence

        # Calculate length: payload size + secondary header - 1 (CCSDS standard)
        payload_size = 46  # 4 timestamp + 16 floats + 2 status + 24 padding
        sec_hdr_size = 10  # Secondary header size
        length = payload_size + sec_hdr_size - 1

        # Pack CCSDS primary header (big-endian)
        ccsds_primary = struct.pack('>HHH', stream_id, sequence, length)

        # Pack CCSDS secondary header (timestamp)
        current_time = int(time.time())
        subseconds = int((time.time() - current_time) * 65536)  # Convert fraction to subseconds
        ccsds_secondary = struct.pack('>IHI', current_time, subseconds, 0)  # seconds, subseconds, spare

        # Pack sensor data payload (matches telemetry structure)
        payload = struct.pack('>Iffff',
                             current_time,  # Timestamp in payload too
                             sensor_data.get('temperature', 0.0),
                             sensor_data.get('cpu_usage', 0.0),
                             sensor_data.get('memory_usage', 0.0),
                             sensor_data.get('gpio_status', 0))

        # Add status bytes and padding
        payload += struct.pack('BB',
                              sensor_data.get('camera_active', 0),
                              sensor_data.get('network_status', 1))
        payload += bytes(24)  # Padding to match expected size

        # Construct complete CCSDS message
        app_msg = ccsds_primary + ccsds_secondary + payload

        # Send as SBN application message
        header = self._pack_sbn_header(self.SBN_APP_MSG, len(app_msg))
        self.socket.send(header + app_msg)
        logger.info(f"Sent sensor data via SBN (MID 0x{self.RASPBERRY_PI_DATA_IN_MID:04X})")

    def _get_next_sequence(self) -> int:
        """Get next sequence number for CCSDS packets"""
        if not hasattr(self, '_sequence_counter'):
            self._sequence_counter = 0
        self._sequence_counter = (self._sequence_counter + 1) & 0x3FFF
        return self._sequence_counter

    def _receive_loop(self):
        """Main receive loop (runs in separate thread)"""
        buffer = b''

        while self.running:
            try:
                # Receive data
                data = self.socket.recv(4096)
                if not data:
                    logger.warning("Connection closed by server")
                    break

                buffer += data

                # Process complete messages
                while len(buffer) >= 7:  # Minimum header size
                    # Parse header
                    msg_sz, msg_type, proc_id = struct.unpack('>HBI', buffer[:7])

                    # Check if we have complete message
                    if len(buffer) < 7 + msg_sz:
                        break  # Wait for more data

                    # Extract message
                    payload = buffer[7:7+msg_sz]
                    buffer = buffer[7+msg_sz:]

                    # Handle message
                    self._handle_message(msg_type, proc_id, payload)

            except Exception as e:
                logger.error(f"Receive error: {e}")
                break

        self.connected = False

    def _handle_message(self, msg_type: int, proc_id: int, payload: bytes):
        """Handle received message"""
        logger.debug(f"Received message type 0x{msg_type:02X} from processor {proc_id}")

        if msg_type == self.SBN_APP_MSG:
            self._handle_app_message(payload)

    def _handle_app_message(self, payload: bytes):
        """Handle standard cFS application message with proper CCSDS parsing"""
        if len(payload) < 6:
            return

        # Parse CCSDS primary header
        stream_id, sequence, length = struct.unpack('>HHH', payload[:6])

        # Mask out version/type bits to get clean MID
        clean_stream_id = stream_id & 0x07FF
        has_secondary_hdr = bool(stream_id & 0x0800)

        msg_offset = 6

        # Skip secondary header if present
        if has_secondary_hdr and len(payload) >= 16:
            # Secondary header: 4 bytes seconds + 2 bytes subseconds + 4 bytes spare
            msg_offset = 16

        msg_data = payload[msg_offset:]
        logger.info(f"Received app message MID 0x{clean_stream_id:04X} (raw: 0x{stream_id:04X})")

        # Handle commands from FSW (check both raw and clean stream ID)
        if clean_stream_id == self.RASPBERRY_PI_DATA_OUT_MID or stream_id == self.RASPBERRY_PI_DATA_OUT_MID:
            self._handle_fsw_command(msg_data)

    def _handle_fsw_command(self, data: bytes):
        """Handle command from FSW"""
        if len(data) < 1:
            return

        cmd_type = data[0] if len(data) > 0 else 0
        logger.info(f"Received FSW command type: {cmd_type}")

        # Process based on command type
        if cmd_type == 1:  # Request status
            self._send_status()
        elif cmd_type == 2:  # Enable camera
            self._enable_camera()
        elif cmd_type == 3:  # Disable camera
            self._disable_camera()

    def _send_status(self):
        """Send status telemetry"""
        # Collect system status
        status_data = {
            'temperature': self._get_cpu_temperature(),
            'cpu_usage': self._get_cpu_usage(),
            'memory_usage': self._get_memory_usage(),
            'gpio_status': self._get_gpio_status(),
            'camera_active': self._is_camera_active(),
            'network_status': 1  # Connected
        }
        self.send_sensor_data(status_data)

    def _get_cpu_temperature(self) -> float:
        """Get CPU temperature (Raspberry Pi specific)"""
        try:
            with open('/sys/class/thermal/thermal_zone0/temp', 'r') as f:
                temp = float(f.read()) / 1000.0
                return temp
        except:
            # Return simulated temperature for testing
            import random
            return 25.0 + random.random() * 20.0

    def _get_cpu_usage(self) -> float:
        """Get CPU usage percentage"""
        try:
            import psutil
            return psutil.cpu_percent()
        except:
            import random
            return random.random() * 100.0

    def _get_memory_usage(self) -> float:
        """Get memory usage percentage"""
        try:
            import psutil
            return psutil.virtual_memory().percent
        except:
            import random
            return random.random() * 100.0

    def _get_gpio_status(self) -> int:
        """Get GPIO pin status"""
        # This would read actual GPIO pins on real Pi
        return 0x00000000

    def _is_camera_active(self) -> bool:
        """Check if camera module is active"""
        # This would check camera status on real Pi
        return False

    def _enable_camera(self):
        """Enable camera module"""
        logger.info("Camera enable command received")
        # Implementation would enable Pi camera

    def _disable_camera(self):
        """Disable camera module"""
        logger.info("Camera disable command received")
        # Implementation would disable Pi camera


# Example usage
if __name__ == "__main__":
    import sys

    # Get NOS3 host from command line or use default
    host = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"

    # Create and connect client
    client = SBNClient(host)

    if client.connect():
        try:
            # Main loop - send sensor data periodically
            while True:
                # Send status telemetry
                client._send_status()
                time.sleep(5)  # Send every 5 seconds

        except KeyboardInterrupt:
            logger.info("Shutting down...")
        finally:
            client.disconnect()
    else:
        logger.error("Failed to connect to SBN")
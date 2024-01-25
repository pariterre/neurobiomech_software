from enum import Enum
import logging
import socket
import threading
from typing import override

from .runner_generic import RunnerGeneric

logger = logging.getLogger("lokomat_fes")


class AcquisitionCommand(Enum):
    START = 1
    EXIT = 2


class RunnerTcpip(RunnerGeneric):
    """Runner that connects to an external software (e.g. GUI) by TCP/IP."""

    def __init__(self, ip_address: str = "localhost", port: int = 4042, *args, **kwargs) -> None:
        """Initialize the Runner.

        Args:
            port: Port to listen on.
        """
        super().__init__(*args, **kwargs)

        self._ip_address = ip_address
        self._port = port
        self._server_socket = None
        self._client_socket = None

    def _start_connection(self):
        """Start the TCP/IP connection."""
        self._server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server_socket.bind((self._ip_address, self._port))
        self._server_socket.listen(1)

        logger.info(f"Waiting for connection on port {self._port}...")
        self._client_socket, addr = self._server_socket.accept()
        logger.info(f"Connected to {addr}")

    def receive_acquisition_command(self):
        """Receive an acquisition command and int value from the external software."""
        data = self._client_socket.recv(1024).decode()
        command_str, value_str = data.split(":")
        command = AcquisitionCommand(int(command_str))
        value = int(value_str)
        logger.info(f"Received command: {command}, value: {value}")
        return command, value

    def _send_acknowledgment(self):
        """Send an acknowledgment back to the external sotware."""
        acknowledgment = "ACK"
        self._client_socket.sendall(acknowledgment.encode())
        logger.info(f"Sent acknowledgment: {acknowledgment}")

    def _close_connection(self):
        """Close the TCP/IP connection."""
        self._client_socket.close()
        self._server_socket.close()
        logger.info("Connection closed")

    @override
    def _exec(self) -> None:
        """Start the runner (internal)."""
        self.start_connection()

        # Start a thread for non-blocking execution
        thread = threading.Thread(target=self.exec_method_non_blocking)
        thread.start()

        while True:
            # Wait for an acquisition command
            command, value = self.receive_acquisition_command()
            logger.info(f"Processing command: {command}, value: {value}")

            if value == AcquisitionCommand.EXIT:
                # Stop the stimulation
                break

            # Send acknowledgment back
            self.send_acknowledgment()

        # Close the connection when done
        self.close_connection()

        logger.info("Starting tcp/ip runner.")

        print("Send 's' to start stimulation, 'q' to stop stimulation: ")
        while True:
            key = input()
            if key == "s":
                duration = input("How long should the stimulation last? (in seconds): ")
                try:
                    duration = float(duration)
                except:
                    logger.exception("Invalid duration.")
                    continue

                self._rehastim.start_stimulation(duration=duration)

            elif key == "q":
                break

        logger.info("Runner tcp/ip exited.")

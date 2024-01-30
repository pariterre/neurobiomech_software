from enum import Enum
import logging
import socket
import time
from typing import override

from .runner_console import RunnerConsole

logger = logging.getLogger("lokomat_fes")
logger_runner = logging.getLogger("runner")


class _Command(Enum):
    START_RECORDING = 1
    STOP_RECORDING = 2
    STIMULATE = 3
    FETCH_DATA = 4
    PLOT_DATA = 5
    SAVE_DATA = 6
    QUIT = 7
    SHUTDOWN = 8

    def __str__(self) -> str:
        if self.name == "START_RECORDING":
            return "start"
        elif self.name == "STOP_RECORDING":
            return "stop"
        elif self.name == "STIMULATE":
            return "stim"
        elif self.name == "FETCH_DATA":
            return "fetch"
        elif self.name == "PLOT_DATA":
            return "plot"
        elif self.name == "SAVE_DATA":
            return "save"
        elif self.name == "QUIT":
            return "quit"
        elif self.name == "SHUTDOWN":
            return "shutdown"
        else:
            raise ValueError(f"Unknown command {self.name}")


class RunnerTcp(RunnerConsole):
    """Runner that connects to an external software (e.g. GUI) by TCP/IP."""

    def __init__(self, ip_address: str = "localhost", port: int = 4042, *args, **kwargs) -> None:
        """Initialize the Runner.

        Parameters
        ----------
        ip_address : str, optional
            IP address to connect to, by default "localhost"
        port : int, optional
            Port to connect to, by default 4042
        """
        super().__init__(*args, **kwargs)

        self._ip_address = ip_address
        self._port = port
        self._server = None
        self._connexion = None

    def _start_connection(self):
        """Start the TCP/IP connection."""
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                self._server.bind((self._ip_address, self._port))
                break
            except OSError:
                logger_runner.error(f"Could not bind to {self._ip_address}:{self._port}. Retrying...")
                time.sleep(1)
                continue

        self._server.listen()
        logger_runner.info(f"Waiting for connection on {self._ip_address}:{self._port}...")
        self._connexion, addr = self._server.accept()
        logger_runner.info(f"Connected to {addr}")

    @override
    def _receive_command(self) -> tuple[str | None, list[str]]:
        """Receive an acquisition command and int value from the external software."""
        logger_runner.info("Waiting for command...")
        try:
            data = self._connexion.recv(1024).decode()
        except Exception:
            data = None

        if not data:
            logger_runner.info("The client has abruptly closed the connection.")
            return None, []

        command_str, parameters_str = data.split(":")
        command = _Command(int(command_str))
        parameters = parameters_str.split(",") if parameters_str else []

        message = f"Received command: {command}"
        if parameters:
            message += f", parameters: {parameters}"
        logger_runner.info(message)

        return str(command), parameters

    def _send_acknowledgment(self, response):
        """Send an acknowledgment back to the external software."""
        acknowledgment = "OK" if response else "ERROR"
        try:
            self._connexion.sendall(acknowledgment.encode())
        except ...:
            logger_runner.error(f"Connection closed by the client.")
            return False

        logger_runner.info(f"Sent acknowledgment: {acknowledgment}")
        return True

    def _close_connection(self):
        """Close the TCP/IP connection."""
        # Give some time to the external software to close the connection
        time.sleep(1)

        self._connexion.close()
        self._server.close()
        logger_runner.info("Connection closed")

    @override
    def _exec(self) -> None:
        # Start the runner (internal).
        # The outer loop is to handle the case where the connection is closed by the external software, in which case
        # we need to wait for a new connection. The inner loop is to handle the case where the external software sends
        # multiple commands in a row.
        while True:
            self._start_connection()

            # Start a thread for non-blocking execution
            while True:
                # Wait for an acquisition command
                command, parameters = self._receive_command()
                if command is None:
                    break

                if command == str(_Command.START_RECORDING):
                    success = self._start_recording_command(parameters)

                elif command == str(_Command.STOP_RECORDING):
                    success = self._stop_recording_command(parameters)

                elif command == str(_Command.STIMULATE):
                    success = self._stimulate_command(parameters)

                elif command == str(_Command.FETCH_DATA):
                    success = self._fetch_data_command(parameters)

                elif command == str(_Command.PLOT_DATA):
                    success = self._plot_data_command(parameters)

                elif command == str(_Command.SAVE_DATA):
                    success = self._save_command(parameters)

                elif command in (str(_Command.QUIT), str(_Command.SHUTDOWN)):
                    # Stop the the server
                    break

                else:
                    logger_runner.error(f"Unknown command {command}")
                    success = False

                # Send acknowledgment back
                if not self._send_acknowledgment(success):
                    break

            # Make sure the devices are stopped
            if self._is_recording:
                self.stop_recording()

            # Close the connection when done
            self._close_connection()

            if command == str(_Command.SHUTDOWN):
                # Trickle down the quit command
                break

        logger_runner.info("Runner tcp exited.")

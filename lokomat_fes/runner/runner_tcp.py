from enum import Enum
import logging
import socket
from typing import override

from .runner_generic import RunnerGeneric

logger = logging.getLogger("lokomat_fes")
logger_runner = logging.getLogger("runner")


class AcquisitionCommand(Enum):
    START_RECORDING = 1
    STOP_RECORDING = 2
    STIMULATE = 3
    PLOT_DATA = 4
    SAVE_DATA = 5
    QUIT = 6


class RunnerTcp(RunnerGeneric):
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
        self._server.bind((self._ip_address, self._port))
        self._server.listen()
        self._connexion, addr = self._server.accept()
        logger_runner.info(f"Connected to {addr}")

    def receive_acquisition_command(self):
        """Receive an acquisition command and int value from the external software."""
        logger_runner.info("Waiting for command...")
        data = self._connexion.recv(1024).decode()
        if not data:
            logger_runner.info("No data received")
            return None, None

        command_str, parameters_str = data.split(":")
        command = AcquisitionCommand(int(command_str))
        parameters = parameters_str.split(",") if parameters_str else []
        logger_runner.info(f"Received command: {command}, value: {parameters}")
        return command, parameters

    def _send_acknowledgment(self, response):
        """Send an acknowledgment back to the external software."""
        acknowledgment = "ACK - OK" if response else "ACK - ERROR"
        self._connexion.sendall(acknowledgment.encode())
        logger_runner.info(f"Sent acknowledgment: {acknowledgment}")

    def _close_connection(self):
        """Close the TCP/IP connection."""
        self._connexion.close()
        self._server.close()
        logger_runner.info("Connection closed")

    @override
    def _exec(self) -> None:
        """Start the runner (internal)."""
        self._start_connection()

        # Start a thread for non-blocking execution
        while True:
            # Wait for an acquisition command
            command, parameters = self.receive_acquisition_command()
            if command is None:
                break

            response = False
            if command == AcquisitionCommand.START_RECORDING:
                # Start the devices
                response = self._start_recording_command(parameters)

            elif command == AcquisitionCommand.STOP_RECORDING:
                # Stop the devices
                response = _try_command(self.stop_recording)

            elif command == AcquisitionCommand.QUIT:
                # Stop the stimulation
                break

            # Send acknowledgment back
            self._send_acknowledgment(response)

        # Close the connection when done
        self._close_connection()

        logger_runner.info("Runner tcp/ip exited.")

    def _start_recording_command(self, parameters: list[str]):
        success = _check_number_parameters("start", parameters, expected=None)
        if not success:
            return False

        success = _try_command(self.start_recording)
        if not success:
            return False

        logger_runner.info("Recording started.")
        return True


def _check_number_parameters(command: str, parameters: list[str], expected: dict[str, bool] | None) -> bool:
    """Check if the number of parameters is correct.

    Parameters
    ----------
    parameters : list[str]
        The parameters to check.
    expected : dict[str, bool]
        The expected parameters, with the key being the name and the value being if it is required or not.
    """
    if expected is None:
        expected = {}

    parameters_names = []
    for key in expected:
        if expected[key]:  # If required
            parameters_names.append(f"{key}")
        else:
            parameters_names.append(f"[{key}]")

    is_required = [required for required in expected.values() if required]
    expected_min = len(is_required)
    expected_max = len(parameters_names)

    if expected_min == expected_max:
        if expected_min == 0:
            error_msg = f"{command} takes no parameters. "
        else:
            error_msg = f"{command} requires {expected_min} parameters. "
    else:
        error_msg = f"{command} requires {expected_min} to {expected_max} parameters. "

    if expected_max > 0:
        error_msg += f"Parameters are: {', '.join(parameters_names)}"

    if len(parameters) < expected_min or len(parameters) > expected_max:
        logger_runner.exception(error_msg)
        return False
    return True


def _parse_float(name: str, value: str) -> float:
    try:
        return float(value)
    except:
        logger_runner.exception(f"Invalid {name}, it must be a float.")
        return None


def _parse_int(name: str, value: str) -> int:
    try:
        return int(value)
    except:
        logger_runner.exception(f"Invalid {name}, it must be an integer.")
        return None


def _try_command(command: str, *args, **kwargs) -> bool:
    try:
        command(*args, **kwargs)
        return True
    except:
        logger_runner.exception(f"Error while executing command {command.__name__}.")
        return False

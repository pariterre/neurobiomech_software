from enum import Enum
import json
import logging
import socket
from struct import pack
import time
from typing import override

from lokomat_fes.common.data import Data

from .runner_console import RunnerConsole

_logger = logging.getLogger("lokomat_fes")


class _Command(Enum):
    START_NIDAQ = 0
    STOP_NIDAQ = 1
    START_RECORDING = 2
    STOP_RECORDING = 3
    STIMULATE = 4
    AVAILABLE_SCHEDULES = 5
    ADD_SCHEDULE = 6
    GET_SCHEDULE = 7
    REMOVE_SCHEDULED = 8
    START_FETCH_DATA = 9
    FETCH_DATA = 10
    PLOT_DATA = 11
    SAVE_DATA = 12
    QUIT = 13
    SHUTDOWN = 14

    def __str__(self) -> str:
        if self.name == "START_NIDAQ":
            return "start_nidaq"
        elif self.name == "STOP_NIDAQ":
            return "stop_nidaq"
        elif self.name == "START_RECORDING":
            return "start"
        elif self.name == "STOP_RECORDING":
            return "stop"
        elif self.name == "STIMULATE":
            return "stim"
        elif self.name == "AVAILABLE_SCHEDULES":
            return "available_schedules"
        elif self.name == "ADD_SCHEDULE":
            return "add_schedule"
        elif self.name == "GET_SCHEDULE":
            return "get_schedule"
        elif self.name == "REMOVE_SCHEDULED":
            return "remove_scheduled"
        elif self.name == "START_FETCH_DATA":
            return "start_fetch"
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

    def __init__(
        self, ip_address: str = "localhost", commandPort: int = 4042, dataPort: int = 4043, *args, **kwargs
    ) -> None:
        """Initialize the Runner.

        Parameters
        ----------
        ip_address : str, optional
            IP address to connect to, by default "localhost"
        commandPort : int, optional
            Port to connect to command channel, by default 4042
        dataPort : int, optional
            Port to connect to data channel, by default 4043
        """
        super().__init__(*args, **kwargs)

        self._ip_address = ip_address
        self._commandPort = commandPort
        self._dataPort = dataPort
        self._commandServer = None
        self._commandConnexion = None
        self._dataServer = None
        self._dataConnexion = None

    def _start_connection(self):
        """Start the TCP/IP connection."""
        self._commandServer, self._commandConnexion = _declare_socket(self._ip_address, self._commandPort)
        self._dataServer, self._dataConnexion = _declare_socket(self._ip_address, self._dataPort)

    @override
    def _receive_command(self) -> tuple[str | None, list[str]]:
        """Receive an acquisition command and int value from the external software."""
        _logger.info("Waiting for command...")
        try:
            data = self._commandConnexion.recv(1024).decode()
        except Exception:
            data = None

        if not data:
            _logger.info("The client has abruptly closed the connection.")
            return None, []

        command_str, parameters_str = data.split(":")
        command = _Command(int(command_str))
        parameters = parameters_str.split(",") if parameters_str else []

        message = f"Received command: {command}"
        if parameters:
            message += f", parameters: {parameters}"
        _logger.info(message)

        return str(command), parameters

    def _send_acknowledgment(self, response):
        """Send an acknowledgment back to the external software."""
        acknowledgment = "OK" if response else "ERROR"
        try:
            self._commandConnexion.sendall(acknowledgment.encode())
        except Exception:
            _logger.error(f"Connection closed by the client.")
            return False

        _logger.info(f"Sent acknowledgment: {acknowledgment}")
        return True

    def _close_connection(self):
        """Close the TCP/IP connection."""
        # Give some time to the external software to close the connection
        time.sleep(1)

        self._commandConnexion.close()
        self._commandServer.close()
        self._dataConnexion.close()
        self._dataServer.close()
        _logger.info("Connection closed")

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

                if command == str(_Command.START_NIDAQ):
                    success = self._start_nidaq_command(parameters)

                elif command == str(_Command.STOP_NIDAQ):
                    success = self._stop_nidaq_command(parameters)

                elif command == str(_Command.START_RECORDING):
                    success = self._start_recording_command(parameters)

                elif command == str(_Command.STOP_RECORDING):
                    success = self._stop_recording_command(parameters)

                elif command == str(_Command.STIMULATE):
                    success = self._stimulate_command(parameters)

                elif command == str(_Command.AVAILABLE_SCHEDULES):
                    success = self._list_available_schedules_command(parameters)

                elif command == str(_Command.ADD_SCHEDULE):
                    success = self._schedule_stimulation_command(parameters)

                elif command == str(_Command.GET_SCHEDULE):
                    success = self._get_scheduled_stimulations_command(parameters)

                elif command == str(_Command.REMOVE_SCHEDULED):
                    success = self._unschedule_stimulation_command(parameters)

                elif command == str(_Command.START_FETCH_DATA):
                    success = self._start_fetch_continuous_data_command(parameters)

                elif command == str(_Command.FETCH_DATA):
                    success = self._fetch_continuous_data_command(parameters)

                elif command == str(_Command.PLOT_DATA):
                    success = self._plot_data_command(parameters)

                elif command == str(_Command.SAVE_DATA):
                    success = self._save_command(parameters)

                elif command in (str(_Command.QUIT), str(_Command.SHUTDOWN)):
                    # Stop the the server
                    break

                else:
                    _logger.error(f"Unknown command {command}")
                    success = False

                # Send acknowledgment back
                if not self._send_acknowledgment(success):
                    break

            # Make sure the devices are stopped
            if self._is_recording:
                self.stop_recording()

            if self._nidaq.is_connected:
                self.stop_nidaq()

            # Close the connection when done
            self._close_connection()

            if command == str(_Command.SHUTDOWN):
                # Trickle down the quit command
                break

        _logger.info("Runner tcp exited.")

    @override
    def _start_nidaq_command(self, parameters: list[str]) -> bool:
        out = super()._start_nidaq_command(parameters)
        if not out:
            return out

        self._dataConnexion.sendall(
            json.dumps(
                {
                    "t0": self._continuous_data.t0.timestamp(),
                    "nidaqNbChannels": self._nidaq.num_channels,
                    "rehastimNbChannels": self._rehastim.nb_channels,
                }
            ).encode()
        )

        return True

    @override
    def _list_available_schedules_command(self, parameters: list[str]) -> bool:
        if not self._check_number_parameters("scheduled_stim", parameters, expected=None):
            return False

        available_schedules = [schedule.serialize() for schedule in self._scheduler.available_schedules]

        self._dataConnexion.sendall(json.dumps(available_schedules).encode())
        return True

    @override
    def _get_scheduled_stimulations_command(self, parameters: list[str]) -> bool:
        if not self._check_number_parameters("scheduled_stim", parameters, expected=None):
            return False

        scheduled_stimulations = [stim.serialize() for stim in self._scheduler.get_stimulations()]
        self._dataConnexion.sendall(json.dumps(scheduled_stimulations).encode())
        return True

    @override
    def _fetch_continuous_data(self, from_top: bool = False) -> Data:
        data = super()._fetch_continuous_data(from_top)

        # Send the message
        self._dataConnexion.sendall(json.dumps(data.serialize(to_json=True)).encode())

        return data


def _declare_socket(ip_address: str, port: int):
    """Declare the socket to be used for the connection."""
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        try:
            server.bind((ip_address, port))
            break
        except OSError:
            _logger.error(f"Could not bind to {ip_address}:{port}. Retrying...")
            time.sleep(1)
            continue

    server.listen()
    _logger.info(f"Waiting for connection on {ip_address}:{port}...")
    connexion, addr = server.accept()
    _logger.info(f"Connected to {addr}")

    return server, connexion

import logging
from typing import override

from .runner_generic import RunnerGeneric

logger = logging.getLogger("lokomat_fes")


class RunnerConsole(RunnerGeneric):
    """Run the program in a local console."""

    @override
    def _exec(self):
        """Start the runner."""
        logger.info("Starting console runner.")

        print("Type your command and press enter, use 'list' the print the commands:")
        while True:
            request = input().lower().split(" ")
            command = request[0]
            parameters = request[1:]

            if command == "list":
                self._list_commands(parameters)

            elif command == "start":
                self._start_recording_command(parameters)

            elif command == "stop":
                self._stop_recording_command(parameters)

            elif command == "stim":
                self._stimulate_command(parameters)

            elif command == "save":
                self._save_command(parameters)

            elif command == "quit":
                break

        logger.info("Runner Console exited.")

    def _list_commands(self, parameters: list[str]):
        success = _check_number_parameters("list", parameters, expected=None)
        if not success:
            return

        print("List of commands:")
        print("\tlist: list all the commands")
        print("\tstart: start recording")
        print("\tstop: stop recording")
        print(
            "\tstim X [Y] [Z]: stimulate for X seconds, at amplitude Y mA, with a width Z ms (default for Y and Z are previously set values, or 0 if not set yet)"
        )
        print("\tsave X: save the last trial to file X as a pickle file")
        print("\tquit: quit")

    def _start_recording_command(self, parameters: list[str]):
        success = _check_number_parameters("start", parameters, expected=None)
        if not success:
            return

        success = _try_command(self._start_recording)
        if not success:
            return
        print("Recording started.")

    def _stop_recording_command(self, parameters: list[str]):
        success = _check_number_parameters("stop", parameters, expected=None)
        if not success:
            return

        success = _try_command(self._stop_recording)
        if not success:
            return
        print("Recording stopped.")

    def _stimulate_command(self, parameters: list[str]):
        success = _check_number_parameters(
            "stim", parameters, expected={"duration": True, "amplitude": False, "width": False}
        )
        if not success:
            return

        if len(parameters) >= 2:
            amplitude = _parse_float("amplitude", parameters[1])
            if amplitude is None:
                return

            success = _try_command(self._set_stimulation_pulse_amplitude, amplitude)
            if not success:
                return

        if len(parameters) >= 3:
            width = _parse_float("width", parameters[2])
            if width is None:
                return

            success = _try_command(self._set_stimulation_pulse_width, width)
            if not success:
                return

        duration = _parse_float("duration", parameters[0])
        if duration is None:
            return

        success = _try_command(self._start_stimulation, duration)
        if not success:
            return
        print(f"Simulating for {duration}s")

    def _save_command(self, parameters: list[str]):
        success = _check_number_parameters("save", parameters, expected={"filename": True})
        if not success:
            return

        filename = parameters[0]
        if not _try_command(self.save_trial, filename):
            return
        print(f"Trial saved to {filename}")


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
        logger.exception(error_msg)
        return False
    return True


def _parse_float(name: str, value: str) -> float:
    try:
        return float(value)
    except:
        logger.exception(f"Invalid {name}, it must be a float.")
        return None


def _try_command(command: str, *args, **kwargs) -> bool:
    try:
        command(*args, **kwargs)
        return True
    except:
        logger.exception(f"Error while executing command {command.__name__}.")
        return False

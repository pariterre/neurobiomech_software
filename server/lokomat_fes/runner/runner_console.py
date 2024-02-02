import logging
from typing import override

from .runner_generic import RunnerGeneric
from ..common.data import Data
from ..scheduler.stimulation import Side, StrideBasedStimulation

_logger = logging.getLogger("lokomat_fes")


_available_schedules = {
    "hip_at_swing_phase": {
        "func": StrideBasedStimulation.stimulate_in_swing_phase,
        "description": "Stimulate when the leg (27% to 56% of the stride) is in swing phase. Second argument is the side (0: left, 1: right, 2: both).",
    },
}


class RunnerConsole(RunnerGeneric):
    """Run the program in a local console."""

    def _receive_command(self) -> tuple[str, list[str]]:
        request = input().lower().split(" ")
        command = request[0]
        parameters = request[1:]
        return command, parameters

    @override
    def _exec(self):
        """Start the runner."""
        _logger.info("Starting console runner.")

        print("Type your command and press enter, use 'list' the print the commands:")
        while True:
            command, parameters = self._receive_command()

            if command == "list":
                self._list_commands(parameters)

            elif command == "start_nidaq":
                self._start_nidaq_command(parameters)

            elif command == "stop_nidaq":
                self._stop_nidaq_command(parameters)

            elif command == "start":
                self._start_recording_command(parameters)

            elif command == "stop":
                self._stop_recording_command(parameters)

            elif command == "available_stim":
                self._list_available_schedules_command(parameters)

            elif command == "scheduled_stim":
                self._print_scheduled_stimulations(parameters)

            elif command == "schedule_stim":
                self._schedule_stimulation_command(parameters)

            elif command == "unschedule_stim":
                self._unschedule_stimulation_command(parameters)

            elif command == "stim":
                self._stimulate_command(parameters)

            elif command == "fetch_data":
                self._fetch_continuous_data_command(parameters)

            elif command == "plot":
                self._plot_data_command(parameters)

            elif command == "save":
                self._save_command(parameters)

            elif command == "quit":
                break

            else:
                _logger.error(f"Unknown command {command}.")

        _logger.info("Runner Console exited.")

    def _list_commands(self, parameters: list[str]):
        if not _check_number_parameters("list", parameters, expected=None):
            return

        print("List of commands:")
        print("\tlist: list all the commands")
        print("\tstart_nidaq: start the nidaq")
        print("\tstop_nidaq: stop the nidaq")
        print("\tstart: start recording")
        print("\tstop: stop recording")
        print("\tavailable_stim: list all the available stimulations")
        print("\tscheduled_stim: list all the scheduled stimulations")
        print("\tschedule_stim X [Y]: schedule stimulation X on side Y (default is both)")
        print("\tunschedule_stim X: unschedule stimulation X")
        print(
            "\tstim X [Y] [Z]: stimulate for X seconds, at amplitude Y mA, with a width Z ms (default for Y and Z are previously set values, or 0 if not set yet)"
        )
        print("\tfetch_data [X]: fetch the data from the last fetch (default) or from the top of the data")
        print(
            "\tplot: plot the last trial, if available. This method is blocking (no other commands can be used while the plot is shown)"
        )
        print("\tsave X: save the last trial to file X as a pickle file")
        print("\tquit: quit")

    def _start_nidaq_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("start_nidaq", parameters, expected=None):
            return False

        return _try_command(self.start_nidaq)

    def _start_recording_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("start", parameters, expected=None):
            return False

        return _try_command(self.start_recording)

    def _stop_nidaq_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("stop_nidaq", parameters, expected=None):
            return False

        return _try_command(self.stop_nidaq)

    def _stop_recording_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("stop", parameters, expected=None):
            return False

        return _try_command(self.stop_recording)

    def _list_available_schedules_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("available_stim", parameters, expected=None):
            return False

        print("List of available schedules:")
        for i, key in enumerate(_available_schedules):
            print(f"\t{i} - {key}: {_available_schedules[key]['description']}")

        return True

    def _schedule_stimulation_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("schedule_stim", parameters, expected={"index": True, "side": False}):
            return False

        index = _parse_int("index", parameters[0])
        if index is None:
            return False
        if index >= len(_available_schedules):
            _logger.error(f"Invalid index, there are only {len(_available_schedules)} available schedules.")
            return False
        key = list(_available_schedules.keys())[index]

        side = Side.BOTH
        if len(parameters) >= 2:
            side_index = _parse_int("side", parameters[1])
            if side_index is None:
                return False
            side = Side(side_index)

        return _try_command(self.schedule_stimulation, _available_schedules[key]["func"](side))

    def _unschedule_stimulation_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("unschedule_stim", parameters, expected={"index": True}):
            return False

        index = _parse_int("index", parameters[0])
        if index is None:
            return False

        stimulations = self.get_scheduled_stimulations()
        if index >= len(stimulations):
            _logger.error(f"Invalid index, there are only {len(stimulations)} stimulations scheduled.")
            return False

        return _try_command(self.remove_scheduled_stimulation, index)

    def _print_scheduled_stimulations(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("scheduled_stim", parameters, expected=None):
            return False

        for i, stim in enumerate(self._scheduler.get_stimulations()):
            print(f"\t{i} - {stim.name}")

        return True

    def _stimulate_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters(
            "stim", parameters, expected={"duration": True, "amplitude": False, "width": False}
        ):
            return False

        if len(parameters) >= 2:
            amplitude = _parse_float("amplitude", parameters[1])
            if amplitude is None:
                return False

            success = _try_command(self.set_stimulation_pulse_amplitude, amplitude)
            if not success:
                return False

        if len(parameters) >= 3:
            width = _parse_int("width", parameters[2])
            if width is None:
                return False

            success = _try_command(self.set_stimulation_pulse_width, width)
            if not success:
                return False

        duration = _parse_float("duration", parameters[0])
        if duration is None:
            return False

        return _try_command(self.start_stimulation, duration)

    def _start_fetch_continuous_data_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("start_fetch_data", parameters, expected=None):
            return False

        return _try_command(self._start_fetch_continuous_data)

    def _fetch_continuous_data_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("fetch_data", parameters, expected={"from_top": False}):
            return False

        from_top = False  # Restart from the top of the data (t0)
        if len(parameters) >= 1:
            from_top = _parse_int("time_index", parameters[0])
            if from_top is None:
                return False
            from_top = bool(from_top)

        return _try_command(self._fetch_continuous_data, from_top)

    def _plot_data_command(self, parameters: list[str]) -> bool:
        if not _check_number_parameters("plot", parameters, expected=None):
            return False

        return self.plot_data()

    def _save_command(self, parameters: list[str]) -> None:
        if not _check_number_parameters("save", parameters, expected={"filename": True}):
            return False

        filename = parameters[0]
        return _try_command(self.save_trial, filename)


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
        _logger.exception(error_msg)
        return False
    return True


def _parse_float(name: str, value: str) -> float:
    try:
        return float(value)
    except:
        _logger.exception(f"Invalid {name}, it must be a float.")
        return None


def _parse_int(name: str, value: str) -> int:
    try:
        return int(value)
    except:
        _logger.exception(f"Invalid {name}, it must be an integer.")
        return None


def _try_command(command: str, *args, **kwargs) -> bool:
    try:
        return command(*args, **kwargs) or True
    except:
        _logger.exception(f"Error while executing command {command.__name__}.")
        return False

import logging
from typing import override

import matplotlib.pyplot as plt

from .runner_generic import RunnerGeneric
from ..common.data import Data
from ..scheduler.stimulation import Side, StrideBasedStimulation

logger = logging.getLogger("lokomat_fes")
logger_runner = logging.getLogger("runner")


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
        logger.info("Starting console runner.")

        print("Type your command and press enter, use 'list' the print the commands:")
        while True:
            command, parameters = self._receive_command()

            if command == "list":
                self._list_commands(parameters)

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

            elif command == "plot":
                self._plot_data_command(parameters)

            elif command == "save":
                self._save_command(parameters)

            elif command == "quit":
                break

            else:
                logger_runner.error(f"Unknown command {command}.")

        logger.info("Runner Console exited.")

    def _list_commands(self, parameters: list[str]):
        success = _check_number_parameters("list", parameters, expected=None)
        if not success:
            return

        print("List of commands:")
        print("\tlist: list all the commands")
        print("\tstart: start recording")
        print("\tstop: stop recording")
        print("\tavailable_stim: list all the available stimulations")
        print("\tscheduled_stim: list all the scheduled stimulations")
        print("\tschedule_stim X [Y]: schedule stimulation X on side Y (default is both)")
        print("\tunschedule_stim X: unschedule stimulation X")
        print(
            "\tstim X [Y] [Z]: stimulate for X seconds, at amplitude Y mA, with a width Z ms (default for Y and Z are previously set values, or 0 if not set yet)"
        )
        print(
            "\tplot: plot the last trial, if available. This method is blocking (no other commands can be used while the plot is shown)"
        )
        print("\tsave X: save the last trial to file X as a pickle file")
        print("\tquit: quit")

    def _start_recording_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("start", parameters, expected=None)
        if not success:
            return False

        success = _try_command(self.start_recording)
        if not success:
            return False

        logger_runner.info("Recording started.")
        return True

    def _stop_recording_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("stop", parameters, expected=None)
        if not success:
            return False

        success = _try_command(self.stop_recording)
        if not success:
            return False

        logger_runner.info("Recording stopped.")
        return True

    def _list_available_schedules_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("available_stim", parameters, expected=None)
        if not success:
            return False

        print("List of available schedules:")
        for i, key in enumerate(_available_schedules):
            print(f"\t{i} - {key}: {_available_schedules[key]['description']}")

        return True

    def _schedule_stimulation_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("schedule_stim", parameters, expected={"index": True, "side": False})
        if not success:
            return False

        index = _parse_int("index", parameters[0])
        if index is None:
            return False
        if index >= len(_available_schedules):
            logger_runner.error(f"Invalid index, there are only {len(_available_schedules)} available schedules.")
            return False
        key = list(_available_schedules.keys())[index]

        side = Side.BOTH
        if len(parameters) >= 2:
            side_index = _parse_int("side", parameters[1])
            if side_index is None:
                return False
            side = Side(side_index)

        self.schedule_stimulation(_available_schedules[key]["func"](side))
        logger_runner.info(f"Scheduled stimulation {key} on side {side}.")
        return True

    def _unschedule_stimulation_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("unschedule_stim", parameters, expected={"index": True})
        if not success:
            return False

        index = _parse_int("index", parameters[0])
        if index is None:
            return False

        stimulations = self.get_scheduled_stimulations()
        if index >= len(stimulations):
            logger_runner.error(f"Invalid index, there are only {len(stimulations)} stimulations scheduled.")
            return False

        self.remove_scheduled_stimulation(index)
        logger_runner.info(f"Unscheduled stimulation {index}.")
        return True

    def _print_scheduled_stimulations(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("scheduled_stim", parameters, expected=None)
        if not success:
            return False

        for i, stim in enumerate(self._scheduler.get_stimulations()):
            print(f"\t{i} - {stim.name}")

        return True

    def _stimulate_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters(
            "stim", parameters, expected={"duration": True, "amplitude": False, "width": False}
        )
        if not success:
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

        success = _try_command(self.start_stimulation, duration)
        if not success:
            return False

        amplitude = self._rehastim.get_pulse_amplitude()[0]
        width = self._rehastim.get_pulse_width()[0]
        logger_runner.info(f"Simulating for {duration}s at {amplitude}mA and {width}ms.")
        return True

    def _fetch_data_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("fetch_data", parameters, expected=None)
        if not success:
            return False

        raise NotImplementedError("fetch_data is not implemented yet.")  # Does this make sense for console?
        return True

    @staticmethod
    def plot_data(data: Data) -> None:
        nidaq_data = data.nidaq
        rehastim_data = data.rehastim

        plt.figure("Data against time")

        # On left-hand side axes, plot nidaq data
        color = "blue"
        ax1 = plt.axes()
        ax1.set_xlabel(f"Time [s] (first sample at {nidaq_data.t0})")
        ax1.set_ylabel("Data [mV]", color=color)
        ax1.tick_params(axis="y", labelcolor=color)
        plt.plot(nidaq_data.time, nidaq_data.as_array.T, color=color)

        # On right-hand side axes, plot rehastim data as stair data (from t0 to duration at height of amplitude)
        color = "red"
        ax2 = ax1.twinx()
        ax2.set_ylabel("Amplitude [mA]", color=color)
        ax2.tick_params(axis="y", labelcolor=color)
        all_time = rehastim_data.time
        all_duration = rehastim_data.duration_as_array
        all_amplitude = rehastim_data.amplitude_as_array.T
        for time, duration, amplitude in zip(all_time, all_duration, all_amplitude):
            channel_index = 0  # Only show the first channel as they are all the same (currently)
            plt.plot([time, time + duration], amplitude[[channel_index, channel_index]], color=color)

        plt.show()

    def _plot_data_command(self, parameters: list[str]) -> bool:
        success = _check_number_parameters("plot", parameters, expected=None)
        if not success:
            return False

        data = self.last_trial
        if data is None:
            logger_runner.error("No data to plot.")
            return False

        self.plot_data(data)
        return True

    def _save_command(self, parameters: list[str]) -> None:
        success = _check_number_parameters("save", parameters, expected={"filename": True})
        if not success:
            return False

        filename = parameters[0]
        if not _try_command(self.save_trial, filename):
            return False

        logger_runner.info(f"Trial saved to {filename}")
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
        logger.exception(error_msg)
        return False
    return True


def _parse_float(name: str, value: str) -> float:
    try:
        return float(value)
    except:
        logger.exception(f"Invalid {name}, it must be a float.")
        return None


def _parse_int(name: str, value: str) -> int:
    try:
        return int(value)
    except:
        logger.exception(f"Invalid {name}, it must be an integer.")
        return None


def _try_command(command: str, *args, **kwargs) -> bool:
    try:
        command(*args, **kwargs)
        return True
    except:
        logger.exception(f"Error while executing command {command.__name__}.")
        return False

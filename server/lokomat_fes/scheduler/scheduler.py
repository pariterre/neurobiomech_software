from datetime import datetime
import json
import logging
import os
import threading
import time

from .automatic_stimulation_rule import AutomaticStimulationRule
from ..common.data import Data

logger = logging.getLogger("lokomat_fes")
_mutex = threading.Lock()


class Scheduler:
    def __init__(self, runner, data: Data) -> None:
        """Initialize the scheduler."""
        from ..runner import RunnerGeneric

        self._runner: RunnerGeneric = runner
        self._data = data

        self.available_schedules: list[AutomaticStimulationRule] = _default_schedules(self)
        self._schedules: dict[int, AutomaticStimulationRule] = {}

        # Start a thread that will run the scheduler at each millisecond to check whether to stimulate or not
        self._is_paused = False
        self._exit_flag = False
        self._thread = threading.Thread(target=self._run)
        self._thread.start()

    def __len__(self) -> int:
        """Get the number of stimulations in the scheduler."""
        return len(self._schedules)

    def create_schedule(self, schedule: AutomaticStimulationRule) -> None:
        """Create a new schedule."""
        self.available_schedules.append(schedule)

    def add(self, stimulation: AutomaticStimulationRule) -> None:
        """Add a stimulation to the scheduler."""
        if hash(stimulation) in self._schedules:
            raise ValueError("This stimulation is already scheduled")
        _mutex.acquire()
        self._schedules[hash(stimulation)] = stimulation
        _mutex.release()

    def get_stimulations(self) -> list[AutomaticStimulationRule]:
        """Get a stimulation from the scheduler."""
        return list(self._schedules.values())

    def remove(self, stimulation: AutomaticStimulationRule) -> None:
        """Remove a stimulation from the scheduler."""
        if hash(stimulation) in self._schedules:
            _mutex.acquire()
            del self._schedules[hash(stimulation)]
            _mutex.release()

    def pause(self) -> None:
        """Pause the scheduler."""
        self._is_paused = True

    def resume(self) -> None:
        """Resume the scheduler."""
        self._is_paused = False

    def dispose(self) -> None:
        """Stop the scheduler."""
        self._exit_flag = True
        self._thread.join()

    def _run(self) -> None:
        """Run the scheduler to check whether to stimulate or not."""
        while True:
            if self._exit_flag:
                break

            if self._is_paused:
                time.sleep(0)
                continue

            t = datetime.now().timestamp() - self._data.t0.timestamp()

            _mutex.acquire()
            # Get all the stimulations to check whether to stimulate or not
            amplitudes = [None] * self._runner.nb_channels_rehastim
            for stimulation in self._schedules.values():
                stimulation.stimulation_amplitudes(t, self._data, amplitudes)

            self._runner.set_stimulation_pulse_amplitude(amplitudes=amplitudes)
            if any(amplitudes) is not None:
                logger.info(f"Starting or modifying a stimulation (amplitude 0 acting as stopping the stimulation)")
                self._runner.start_stimulation()

            _mutex.release()
            time.sleep(0)


def _default_schedules(self) -> list[AutomaticStimulationRule]:
    """Get the default schedules."""
    out = []

    # get current file folder
    current_folder = os.path.dirname(os.path.abspath(__file__))

    with open(f"{current_folder}/default_stimulations_schedules_rules.json") as f:
        rules = json.load(f)
        for rule in rules:
            out.append(AutomaticStimulationRule.from_json(rule))
    return out

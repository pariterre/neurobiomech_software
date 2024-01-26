from datetime import datetime
import threading
import time

from .stimulation import StimulationAbstract
from ..common.data import Data


class Planner:
    def __init__(self, runner, data: Data) -> None:
        """Initialize the planner."""
        from ..runner import RunnerGeneric

        self._runner: RunnerGeneric = runner
        self._data = data

        self._plans: dict[int, StimulationAbstract] = {}

        # Start a thread that will run the planner at each millisecond to check whether to stimulate or not
        self._is_paused = False
        self._exit_flag = False
        self._thread = threading.Thread(target=self._run)
        self._thread.start()

    def add(self, stimulation: StimulationAbstract) -> None:
        """Add a stimulation to the planner."""
        self._plans[id(stimulation)] = stimulation

    def get_stimulations(self) -> list[StimulationAbstract]:
        """Get a stimulation from the planner."""
        return list(self._plans.values())

    def remove(self, stimulation: StimulationAbstract) -> None:
        """Remove a stimulation from the planner."""
        if id(stimulation) in self._plans:
            del self._plans[id(stimulation)]

    def pause(self) -> None:
        """Pause the planner."""
        self._is_paused = True

    def resume(self) -> None:
        """Resume the planner."""
        self._is_paused = False

    def dispose(self) -> None:
        """Stop the planner."""
        self._exit_flag = True
        self._thread.join()

    def _run(self) -> None:
        """Run the planner to check whether to stimulate or not."""
        while True:
            if self._exit_flag:
                break

            if self._is_paused:
                time.sleep(0)
                continue

            t = (self._data.t0 - datetime.now()).microseconds / 1e6

            for stimulation in self._plans.values():
                duration = stimulation.stimulation_duration(t, self._data)

                # TODO: See how to implement stimulation on a single channel at a time
                duration_no_none = [d for d in duration if d is not None]
                duration = max(duration_no_none) if duration_no_none else None
                if duration is not None:
                    if duration > 0:
                        print(f"Starting stimulation for {duration} seconds")
                        self._runner.start_stimulation(duration=duration)
                    elif duration == 0:
                        print(f"Starting stimulation indefinitely")
                        self._runner.start_stimulation(duration=None)
                    else:
                        print(f"Stopping stimulation")
                        self._runner.stop_stimulation()

            time.sleep(0)

from datetime import datetime
import logging
import threading
import time

from .stimulation import StimulationAbstract
from ..common.data import Data

logger = logging.getLogger("lokomat_fes")
_mutex = threading.Lock()


class Scheduler:
    def __init__(self, runner, data: Data) -> None:
        """Initialize the scheduler."""
        from ..runner import RunnerGeneric

        self._runner: RunnerGeneric = runner
        self._data = data

        self._schedules: dict[int, StimulationAbstract] = {}

        # Start a thread that will run the scheduler at each millisecond to check whether to stimulate or not
        self._is_paused = False
        self._exit_flag = False
        self._thread = threading.Thread(target=self._run)
        self._thread.start()

    def __len__(self) -> int:
        """Get the number of stimulations in the scheduler."""
        return len(self._schedules)

    def add(self, stimulation: StimulationAbstract) -> None:
        """Add a stimulation to the scheduler."""
        if hash(stimulation) in self._schedules:
            raise ValueError("This stimulation is already scheduled")
        _mutex.acquire()
        self._schedules[hash(stimulation)] = stimulation
        _mutex.release()

    def get_stimulations(self) -> list[StimulationAbstract]:
        """Get a stimulation from the scheduler."""
        return list(self._schedules.values())

    def remove(self, stimulation: StimulationAbstract) -> None:
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
            for stimulation in self._schedules.values():
                duration = stimulation.stimulation_duration(t, self._data)

                # TODO: See how to implement stimulation on a single channel at a time
                duration_no_none = [d for d in duration if d is not None]
                duration = max(duration_no_none) if duration_no_none else None
                if duration is not None:
                    if duration > 0:
                        logger.info(f"Starting stimulation for {duration} seconds")
                        self._runner.start_stimulation(duration=duration)
                    elif duration == 0:
                        logger.info(f"Starting stimulation indefinitely")
                        self._runner.start_stimulation(duration=None)
                    else:
                        logger.info(f"Stopping stimulation")
                        self._runner.stop_stimulation()

            _mutex.release()
            time.sleep(0)

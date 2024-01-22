import numpy as np

from .lokomat_nidaq import LokomatNiDaq
from .perpetual_timer import PerpetualTimer


class LokomatNiDaqMock(LokomatNiDaq):
    """Mock class for NiDaq"""

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._timer: PerpetualTimer | None = None
        self._timer_counter: int = 0

    def _setup_task(self):
        pass

    def _start_task(self):
        """Simulate the start of the task by launching a timer that calls the callback function every dt seconds"""
        self._timer = PerpetualTimer(1, self._generate_fake_data)
        self._timer_counter = 0
        self._timer.start()

    def _stop_task(self):
        self._timer.cancel()
        self._timer = None

    def _generate_fake_data(self):
        """Generate fake data and call the callback function"""
        normalized_time = np.linspace(
            self._timer_counter, self._timer_counter + self._time_between_samples, self.frame_rate
        )

        fake_data = np.ndarray((self.num_channels, self.frame_rate)) * np.nan
        # First row is hip angle that resembles a sine wave which takes about 1 second to complete
        fake_data[0, :] = np.sin(2 * np.pi * normalized_time)

        self._data_has_arrived(fake_data)

        self._timer_counter += self._time_between_samples

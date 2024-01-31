from typing import override

import numpy as np

from .lokomat_nidaq import NiDaqLokomat
from .perpetual_timer import PerpetualTimer


class NiDaqLokomatMock(NiDaqLokomat):
    """Mock class for NiDaq"""

    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._timer: PerpetualTimer | None = None
        self._timer_counter: int = 0

    @override
    def _setup_task(self):
        pass

    @override
    def _start_task(self):
        """Simulate the start of the task by launching a timer that calls the callback function every dt seconds"""
        self._timer = PerpetualTimer(self._time_between_samples, self._generate_fake_data)
        self._timer_counter = 0
        self._timer.start()
        self._is_connected = True

    @override
    def _stop_task(self):
        self._timer.cancel()
        self._timer = None
        self._is_connected = False

    def _generate_fake_data(self):
        """Generate fake data and call the callback function, emulating the [_data_has_arrived] method"""
        normalized_time = np.linspace(
            self._timer_counter, self._timer_counter + self._time_between_samples, self._n_samples_per_block
        )

        fake_data = np.ndarray((self.num_channels, self._n_samples_per_block)) * np.nan
        # First row is hip angle that resembles a sine wave which takes about 1 second to complete
        fake_data[0, :] = np.sin(2 * np.pi * normalized_time)

        self._manage_new_data(fake_data)

        self._timer_counter += self._time_between_samples

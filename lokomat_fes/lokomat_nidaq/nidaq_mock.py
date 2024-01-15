from typing import Any, Callable

import numpy as np


class GenericNiDaq:
    def __init__(self, num_channels: int, rate: int, on_data_ready_callback: Callable | None) -> None:
        """
        Parameters
        ----------
        num_channels : int
            Number of channels connected to the NiDaq
        rate : int
            Frames per second
        on_data_ready_callback : Callable
            Callback function that is called when data is ready
        """
        self.num_channels = num_channels
        self.rate = rate

        # Data releated variables
        self._t: list[np.ndarray] = []
        self._samples: list[np.ndarray] = []

        # Callback function that is called when new data are added
        self.on_data_ready_callback = on_data_ready_callback

        # Setup the NiDaq task
        self.task = None
        self._setup_task()

    @property
    def dt(self) -> float:
        """Time between samples"""
        return 1 / self.rate

    def start_recording(self):
        """Start recording"""
        self._reset_data()

    def _reset_data(self):
        """Reset data to start a new trial"""
        self._t = None
        self._samples = None

    def _add_data(self, data):
        """Callback function for reading signals."""
        t0 = 0 if not self._t else (self._t[-1][-1] + self.dt)
        self._t.append(np.linspace(t0, t0 + 1 - self.dt, self.rate))
        self._samples.append(data)

        if self.on_data_ready_callback is not None:
            self.on_data_ready_callback(self._t, self._samples)

    def _setup_task(self):
        import nidaqmx
        from nidaqmx.constants import AcquisitionType

        with nidaqmx.Task() as task:
            for i in range(self.num_channels):
                task.ai_channels.add_ai_voltage_chan(f"cDAQ1Mod1/ai{i}")

            task.timing.cfg_samp_clk_timing(self.rate, sample_mode=AcquisitionType.CONTINUOUS)
            task.register_every_n_samples_acquired_into_buffer_event(
                self.rate,
                lambda _0, _1, _2, _3: self._add_data(self, task.read(number_of_samples_per_channel=self.rate)),
            )

            # run the task
            task.start()
            input("Running task. Press Enter to stop and see number of " "accumulated samples.\n")
            task.stop()


class LokomatNiDaq(GenericNiDaq):
    def __init__(self, on_data_ready_callback: Callable) -> None:
        super().__init__(num_channels=25, rate=1000, on_data_ready_callback=on_data_ready_callback)


class LokomatNiDaqMock(LokomatNiDaq):
    """Mock class for NiDaq"""

    def __init__(self, on_data_ready_callback: Callable) -> None:
        super().__init__(on_data_ready_callback=on_data_ready_callback)

    def _setup_task(self):
        import numpy as np

        self._t = np.linspace(0, 1, 1000)
        self._samples = np.random.rand(25, 1000)

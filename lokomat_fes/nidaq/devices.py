from abc import ABC, abstractmethod
from typing import Callable, Any

import numpy as np

from .data import NiDaqData


class NiDaqGeneric(ABC):
    def __init__(self, num_channels: int, frame_rate: int) -> None:
        """
        Parameters
        ----------
        num_channels : int
            Number of channels connected to the NiDaq
        rate : int
            Frames per second
        """
        self._num_channels = num_channels
        self._frame_rate = frame_rate
        self._time_between_samples: float = 1  # second

        # Data releated variables
        self._data: NiDaqData = NiDaqData()

        # Callback function that is called when new data are added
        self._is_recording: bool = False
        self._on_start_recording_callback: dict[Any, Callable[[], None]] = {}
        self._on_data_ready_callback: dict[Any, Callable[[np.ndarray, np.ndarray], None]] = {}
        self._on_stop_recording_callback: dict[Any, Callable[[NiDaqData], None]] = {}

        # Setup the NiDaq task
        self._task = None
        self._setup_task()

    @property
    def num_channels(self) -> int:
        """Number of channels connected to the NiDaq"""
        return self._num_channels

    @property
    def frame_rate(self) -> int:
        """Frames per second"""
        return self._frame_rate

    @property
    def dt(self) -> float:
        """Time between samples"""
        return 1 / self.frame_rate

    def register_to_start_recording(self, callback: Callable[[], None]) -> None:
        """Register a callback function that is called when the recording starts"""
        self._on_start_recording_callback[id(callback)] = callback

    def unregister_to_start_recording(self, callback: Callable[[], None]) -> None:
        """Unregister a callback function that is called when the recording starts"""
        del self._on_start_recording_callback[id(callback)]

    def register_to_data_ready(self, callback: Callable[[np.ndarray, np.ndarray], None]) -> None:
        """Register a callback function that is called when new data are ready"""
        self._on_data_ready_callback[id(callback)] = callback

    def unregister_to_data_ready(self, callback: Callable[[np.ndarray, np.ndarray], None]) -> None:
        """Unregister a callback function that is called when new data are ready"""
        del self._on_data_ready_callback[id(callback)]

    def register_to_stop_recording(self, callback: Callable[[NiDaqData], None]) -> None:
        """Register a callback function that is called when the recording stops"""
        self._on_stop_recording_callback[id(callback)] = callback

    def unregister_to_stop_recording(self, callback: Callable[[NiDaqData], None]) -> None:
        """Unregister a callback function that is called when the recording stops"""
        del self._on_stop_recording_callback[id(callback)]

    def start_recording(self) -> None:
        """Start recording"""
        if self._is_recording:
            raise RuntimeError("Already recording")

        for key in self._on_start_recording_callback.keys():
            self._on_start_recording_callback[key]()

        self._reset_data()
        self._start_task()
        self._is_recording = True

    def stop_recording(self) -> None:
        """Stop recording"""
        if not self._is_recording:
            # If we are not currently recording, we don't need to stop the recording
            return

        for key in self._on_stop_recording_callback.keys():
            self._on_stop_recording_callback[key](self._data)

        self._stop_task()
        self._is_recording = False

    @property
    def data(self) -> NiDaqData:
        """Data from the NiDaq"""
        return self._data.copy

    def dispose(self) -> None:
        """Dispose the NiDaq class"""
        self.stop_recording()
        if self._task is not None:
            self._task.close()
            self._task = None

    def _reset_data(self) -> None:
        """Reset data to start a new trial"""
        self._data = NiDaqData()

    def _data_has_arrived(self, data: np.ndarray) -> int:
        """
        Callback function for reading signals.
        It automatically computes the time vector and calls the callback function if it exists.
        """

        prev_t, _ = self._data.sample_block(index=-1, unsafe=True)
        t0 = 0 if prev_t is None else (prev_t[-1] + self.dt)
        t = np.linspace(t0, t0 + self._time_between_samples - self.dt, self.frame_rate)

        self._data.add(t, data)

        if self._on_data_ready_callback:
            data_for_callbacks = self._data.sample_block(index=-1, unsafe=True)
            for key in self._on_data_ready_callback.keys():
                self._on_data_ready_callback[key](*data_for_callbacks)

        return 1  # Success

    def _setup_task(self) -> None:
        """Setup the NiDaq task"""
        import nidaqmx
        from nidaqmx.constants import AcquisitionType

        self._task = nidaqmx.Task()  # This replaces the usual with statement
        for i in range(self.num_channels):
            self._task.ai_channels.add_ai_voltage_chan(self._channel_name(i))

        self._task.timing.cfg_samp_clk_timing(self.frame_rate, sample_mode=AcquisitionType.CONTINUOUS)

        n_samples = self._time_between_samples * self.frame_rate
        self._task.register_every_n_samples_acquired_into_buffer_event(
            n_samples,
            lambda *_, **__: self._data_has_arrived(np.array(self._task.read(number_of_samples_per_channel=n_samples))),
        )

    @abstractmethod
    def _channel_name(self, channel: int) -> str:
        """Channel name assuming it is the channel number [channel]"""

    def _start_task(self) -> None:
        """Start the NiDaq task"""
        self._task.start()

    def _stop_task(self) -> None:
        self._task.stop()

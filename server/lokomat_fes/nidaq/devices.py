from abc import ABC, abstractmethod
from datetime import datetime
import threading
from typing import Callable, Any

import numpy as np

from .data import NiDaqData

_mutex = threading.Lock()


class NiDaqGeneric(ABC):
    def __init__(self, num_channels: int, frame_rate: int, time_between_samples: int = 1) -> None:
        """
        Parameters
        ----------
        num_channels : int
            Number of channels connected to the NiDaq
        rate : int
            Frames per second
        time_between_samples : int
            Time between samples in seconds (this determines the number of samples per frame)
        """
        self._num_channels = num_channels

        self._frame_rate = frame_rate  # Frames per second (Hz)
        self._time_between_samples = time_between_samples  # Time between block of samples in seconds
        self._n_samples_per_block = int(self._time_between_samples * self._frame_rate)  # Number of samples per block

        # Data releated variables
        self._data: NiDaqData = None

        # Callback function that is called when new data are added
        self._is_recording: bool = False
        self._on_start_recording_callback: dict[Any, Callable[[], None]] = {}
        self._on_data_ready_callback: dict[Any, Callable[[np.ndarray, np.ndarray], None]] = {}
        self._on_stop_recording_callback: dict[Any, Callable[[NiDaqData], None]] = {}

        # Setup the NiDaq task
        self._is_connected = False
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
        if hash(callback) not in self._on_start_recording_callback:
            _mutex.acquire()
            self._on_start_recording_callback[hash(callback)] = callback
            _mutex.release()

    def unregister_to_start_recording(self, callback: Callable[[], None]) -> None:
        """Unregister a callback function that is called when the recording starts"""

        if hash(callback) in self._on_start_recording_callback:
            _mutex.acquire()
            del self._on_start_recording_callback[hash(callback)]
            _mutex.release()

    def register_to_data_ready(self, callback: Callable[[np.ndarray, np.ndarray], None]) -> None:
        """Register a callback function that is called when new data are ready"""
        if hash(callback) not in self._on_data_ready_callback:
            _mutex.acquire()
            self._on_data_ready_callback[hash(callback)] = callback
            _mutex.release()

    def unregister_to_data_ready(self, callback: Callable[[np.ndarray, np.ndarray], None]) -> None:
        """Unregister a callback function that is called when new data are ready"""
        if hash(callback) in self._on_data_ready_callback:
            _mutex.acquire()
            del self._on_data_ready_callback[hash(callback)]
            _mutex.release()

    def register_to_stop_recording(self, callback: Callable[[NiDaqData], None]) -> None:
        """Register a callback function that is called when the recording stops"""
        if hash(callback) not in self._on_stop_recording_callback:
            _mutex.acquire()
            self._on_stop_recording_callback[hash(callback)] = callback
            _mutex.release()

    def unregister_to_stop_recording(self, callback: Callable[[NiDaqData], None]) -> None:
        """Unregister a callback function that is called when the recording stops"""
        if hash(callback) in self._on_stop_recording_callback:
            _mutex.acquire()
            del self._on_stop_recording_callback[hash(callback)]
            _mutex.release()

    @property
    def is_connected(self) -> bool:
        """Whether the NiDaq is connected"""
        return self._is_connected

    def connect(self) -> None:
        """Connect the NiDaq"""
        if self._is_connected:
            raise RuntimeError("Cannot connect the device while it is already connected")
        self._data = NiDaqData()
        self._start_task()
        self._is_connected = True

    def disconnect(self) -> None:
        """Disconnect the NiDaq"""
        if not self._is_connected:
            raise RuntimeError("Cannot disconnect the device while it is not connected")
        self._stop_task()
        self._is_connected = False

    def start_recording(self) -> None:
        """Start recording"""
        if not self._is_connected:
            raise RuntimeError("Cannot start recording without the device being connected")

        if self._is_recording:
            raise RuntimeError("Cannot start recording while already recording")

        _mutex.acquire()
        for key in self._on_start_recording_callback.keys():
            self._on_start_recording_callback[key]()
        _mutex.release()

        self._reset_data()
        self._is_recording = True

    def stop_recording(self) -> None:
        """Stop recording"""
        if not self._is_recording:
            # If we are not currently recording, we don't need to stop the recording
            return

        _mutex.acquire()
        for key in self._on_stop_recording_callback.keys():
            self._on_stop_recording_callback[key](self._data)
        _mutex.release()

        self._is_recording = False

    @property
    def data(self) -> NiDaqData:
        """Data from the NiDaq"""
        return self._data.copy

    def dispose(self) -> None:
        """Dispose the NiDaq class"""
        self.stop_recording()
        if self._task is not None:
            self.disconnect()
            self._task.close()
            self._task = None

    def _reset_data(self) -> None:
        """Reset data to start a new trial"""
        self._data = NiDaqData()

    def _data_has_arrived(self, task_handle: int, event_type: int, num_samples: int, callback_data: Any) -> int:
        """Callback function for reading signals"""
        try:
            data = self._task.read(number_of_samples_per_channel=self._n_samples_per_block)
        except:
            return 0
        self._manage_new_data(np.array(data))
        return 1  # Success

    def _manage_new_data(self, data: np.ndarray) -> int:
        """
        Callback function for reading signals.
        It automatically computes the time vector and calls the callback function if it exists.
        """

        n_frames = int(self.frame_rate * self._time_between_samples)
        dt = self.dt  # This is so we finish the time vector one dt before the next sample

        prev_t, _ = self._data.sample_block(index=-1, unsafe=True)
        t0 = datetime.now().timestamp() - self._time_between_samples if prev_t is None else (prev_t[-1] + dt)
        t = np.linspace(t0, t0 + self._time_between_samples - dt, n_frames)

        self._data.add(t, data)

        if self._on_data_ready_callback:
            data_for_callbacks = self._data.sample_block(index=-1, unsafe=True)
            _mutex.acquire()
            for key in self._on_data_ready_callback.keys():
                self._on_data_ready_callback[key](*data_for_callbacks)
            _mutex.release()

    def _setup_task(self) -> None:
        """Setup the NiDaq task"""
        import nidaqmx
        from nidaqmx.constants import AcquisitionType

        self._task = nidaqmx.Task()  # This replaces the usual with statement
        for i in range(self.num_channels):
            self._task.ai_channels.add_ai_voltage_chan(self._channel_name(i))

        self._task.timing.cfg_samp_clk_timing(self.frame_rate, sample_mode=AcquisitionType.CONTINUOUS)

        self._task.register_every_n_samples_acquired_into_buffer_event(
            self._n_samples_per_block,
            self._data_has_arrived,
        )

    @abstractmethod
    def _channel_name(self, channel: int) -> str:
        """Channel name assuming it is the channel number [channel]"""

    def _start_task(self) -> None:
        """Start the NiDaq task"""
        self._task.start()

    def _stop_task(self) -> None:
        self._task.stop()

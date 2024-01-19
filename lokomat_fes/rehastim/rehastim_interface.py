from abc import ABC, abstractproperty, abstractstaticmethod, abstractmethod
from enum import Enum
import multiprocessing as mp
import time

from pyScienceMode import Channel, RehastimGeneric


class RehastimDeviceEvent(Enum):
    """The events that can be triggered by the device."""

    STOP = "stop"
    STIMULATION_DURATION = "stimulation duration"


class RehastimDeviceAbstract(ABC):
    """
    This device mechanism serves as a standardisation layer to account for discrepancies of the implementation of the
    devices in the pyScienceMode library.
    """

    def __init__(self, port: str, channels: list[Channel], low_frequency_factor: int, show_log: bool = False):
        self.port = port
        self.show_log = show_log

        self._pysciencemode_device = self._to_sciencemode()
        self._init_channel(channels=channels, low_frequency_factor=low_frequency_factor)

        self._events = mp.Manager().Value("events", self._empty_events)
        self._stimulation_process = mp.Process(target=self._wait_for_stimulation_request, args=(self._events,))

    @abstractstaticmethod
    def get_name():
        """Get the name of the device."""

    def start_device(self):
        if self._stimulation_process.is_alive():
            raise RuntimeError("The device is already started.")

        """Start the device."""
        self._stimulation_process.start()

    def perform_stimulation(self, duration: float):
        """Perform a stimulation.

        Parameters
        ----------
        duration : float
            The duration of the stimulation in seconds.
        """
        if not self._stimulation_process.is_alive():
            raise RuntimeError("The device is not running, please call [start_device] before.")

        self._set_event(self._events, RehastimDeviceEvent.STIMULATION_DURATION, duration)

    def stop_device(self):
        """Stop the device."""
        if not self._stimulation_process.is_alive():
            return

        self._set_event(self._events, RehastimDeviceEvent.STOP, True)

    def dispose(self):
        """Dispose the device."""
        if not self._stimulation_process.is_alive():
            return

        self.stop_device()
        self._stimulation_process.join()

    def _wait_for_stimulation_request(self, events):
        """Wait for a stimulation to finish.

        Parameters
        ----------
        duration_events : dict[str, float]
            The duration of the stimulation in seconds.
        """

        while True:
            last_events = self._pop_events(events)

            if last_events[RehastimDeviceEvent.STOP]:
                break

            if last_events[RehastimDeviceEvent.STIMULATION_DURATION]:
                duration = last_events[RehastimDeviceEvent.STIMULATION_DURATION]
                self._pysciencemode_device.start_stimulation(stimulation_duration=duration)

            time.sleep(0)

    @staticmethod
    def _set_event(events, new_event, value):
        out = events.value
        out[new_event] = value
        events.set(out)

    def _pop_events(self, events):
        out = events.value
        events.set(self._empty_events)
        return out

    @property
    def _empty_events(self):
        return {
            RehastimDeviceEvent.STOP: False,
            RehastimDeviceEvent.STIMULATION_DURATION: None,
        }

    @abstractmethod
    def _to_sciencemode(self) -> RehastimGeneric:
        """Convert to a pyScienceMode device."""

    @abstractmethod
    def _init_channel(self, channels: list[Channel], low_frequency_factor: int):
        """Initialize the channels."""


class Rehastim2Device(RehastimDeviceAbstract):
    @staticmethod
    def get_name():
        return "Rehastim2"

    def _to_sciencemode(self):
        from pyScienceMode.rehastim2 import Rehastim2

        return Rehastim2(port=self.port, show_log=self.show_log)

    def _init_channel(self, channels: list[Channel], low_frequency_factor: int):
        self._pysciencemode_device.init_channel(
            stimulation_interval=200, list_channels=channels, low_frequency_factor=low_frequency_factor
        )


class RehastimP24Device(RehastimDeviceAbstract):
    @staticmethod
    def get_name():
        return "RehastimP24"

    def _to_sciencemode(self):
        from pyScienceMode import RehastimP24

        return RehastimP24(port=self.port, show_log=self.show_log)

    def _init_channel(self, channels: list[Channel], low_frequency_factor: int):
        self._pysciencemode_device.init_channel(list_channels=channels)


class RehastimInterface(ABC):
    def __init__(self, channels: list[Channel], port: str, low_frequency_factor: int):
        """
        Parameters
        ----------
        device : T
            The device to use.
        channels : list[Channel]
            The channels to use.
        port : str
            The port on which the stimulator is connected.
        """

        self._device: RehastimDeviceAbstract = self._chosen_device(
            port=port, channels=channels, low_frequency_factor=low_frequency_factor, show_log=False
        )

    def start_device(self):
        """Start the device."""
        self._device.start_device()

    def perform_stimulation(self, duration: float):
        self._device.perform_stimulation(duration=duration)

    def stop_device(self):
        """Stop the device."""
        self._device.stop_device()

    def dispose(self):
        """Dispose the device."""
        self._device.dispose()

    @abstractproperty
    def _chosen_device(self) -> type[RehastimDeviceAbstract]:
        """The device to use."""
        pass

    @property
    def device_name(self) -> str:
        return self._chosen_device.get_name()

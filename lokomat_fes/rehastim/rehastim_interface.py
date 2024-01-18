from abc import ABC, abstractproperty, abstractstaticmethod, abstractmethod
import multiprocessing as mp

from pyScienceMode import Channel, RehastimGeneric


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

        self._stimulate_events = {}
        mp.Process(target=self._wait_for_stimulation_request, args=(self._stimulate_events,)).start()

    @abstractstaticmethod
    def get_name():
        """Get the name of the device."""

    def dispose(self):
        """Dispose the device."""

    def _wait_for_stimulation_request(self, duration: float):
        """Wait for a stimulation to finish.

        Parameters
        ----------
        duration : float
            The duration of the stimulation in seconds.
        """
        # self._event.wait()
        # time.sleep(duration)
        # self._event.clear()

    def perform_stimulation(self, duration: float):
        """Perform a stimulation.

        Parameters
        ----------
        duration : float
            The duration of the stimulation in seconds.
        """
        mp.Process(target=self._pysciencemode_device.start_stimulation, args=(duration,)).start()

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

    def start(self):
        self._pysciencemode_device.start_stimulation()


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

        self._device = self._chosen_device(
            port=port, channels=channels, low_frequency_factor=low_frequency_factor, show_log=False
        )

    def dispose(self):
        self._device.dispose()

    def perform_stimulation(self, duration: float):
        self._device.perform_stimulation(duration=duration)

    @abstractproperty
    def _chosen_device(self) -> type[RehastimDeviceAbstract]:
        """The device to use."""
        pass

    @property
    def device_name(self) -> str:
        return self._chosen_device.get_name()

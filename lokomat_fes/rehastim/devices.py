from threading import Timer
from typing import override, Any, Callable
from abc import ABC, abstractproperty, abstractmethod

from pyScienceMode import Channel, RehastimGeneric as pyScienceModeRehastimGeneric


class RehastimGeneric(ABC):
    """
    This device mechanism serves as a standardisation layer to account for discrepancies of the implementation of the
    devices in the pyScienceMode library.
    """

    def __init__(self, port: str, show_log: bool = False) -> None:
        self.port = port
        self.show_log = show_log

        self._device = self._get_initialized_device()
        self._on_stimulation_started_callback: dict[Any, Callable[[], None]] = {}
        self._is_stimulation_initialized = False

    @abstractproperty
    def device_name(self) -> str:
        """Get the name of the device."""

    @abstractproperty
    def nb_channels(self) -> int:
        """Get the number of channels of the device."""

    def register_to_on_stimulation_started(self, callback: Callable[[float, tuple[Channel, ...] | None], None]) -> None:
        """Register a callback function that is called when the stimulation starts
        The callback function takes two arguments:
        - duration: the duration of the stimulation in seconds
        - channels: the channels that are stimulated. If None is sent for the channels, it means that the channels have
        not changed since the last stimulation (so the user can assume that the channels are the same as the last)
        """
        self._on_stimulation_started_callback[id(callback)] = callback

    def unregister_to_on_stimulation_started(
        self, callback: Callable[[float, tuple[Channel, ...] | None], None]
    ) -> None:
        """Unregister a callback function that is called when the stimulation starts"""
        if id(callback) in self._on_stimulation_started_callback:
            del self._on_stimulation_started_callback[id(callback)]

    def start_stimulation(self, duration: float = None) -> None:
        """Perform a stimulation.

        Parameters
        ----------
        duration : float
            The duration of the stimulation in seconds. If None, the stimulation will be performed up to the call of
            [stop_stimulation].
        """
        channels = self._get_channel_list_for_stimulation()

        self._device.start_stimulation(upd_list_channels=channels)

        # Notify the listeners that the stimulation is starting
        channels = self._get_channels()
        for callback in self._on_stimulation_started_callback.values():
            callback(duration, channels)

        if duration is not None:
            Timer(duration, self.stop_stimulation).start()

    @abstractmethod
    def set_pulse_amplitude(self, amplitudes: float | list[float]) -> None:
        """Set the amplitude of the channels.

        Parameters
        ----------
        amplitudes : float | list[float]
            The amplitude of the channels. If a float is given, then all the channels will have the same amplitude.
        """

    @abstractmethod
    def get_pulse_amplitude(self) -> list[float]:
        """Get the amplitude of the channels."""

    @abstractmethod
    def set_pulse_width(self, widths: int | list[int]) -> None:
        """Set the width of the stimulation.

        Parameters
        ----------
        widths : int | list[int]
            The width of the stimulation in milliseconds. If a int is given, then all the channels will have the same width.
        """

    @abstractmethod
    def get_pulse_width(self) -> list[int]:
        """Get the width of the stimulation."""

    @abstractmethod
    def set_pulse_interval(self, interval: float) -> None:
        """Set the interval between the stimulations.

        Parameters
        ----------
        interval : float
            The interval between the stimulations in milliseconds.
        """

    @abstractmethod
    def reset_pulse_values_to_default(self) -> None:
        """Reset the stimulation to the default values."""

    @abstractmethod
    def _get_channel_list_for_stimulation(self) -> list[Channel] | None:
        """Get the channel list for the stimulation. If None is returned, then the stimulation will be the same as the
        last one."""

    def stop_stimulation(self) -> None:
        """Pause the stimulation."""
        if not self._is_stimulation_initialized:
            return
        self._device.pause_stimulation()

    def dispose(self) -> None:
        """Dispose the device."""
        self._device.end_stimulation()
        self._device.disconnect()
        self._device.close_port()

    @abstractmethod
    def _get_initialized_device(self) -> pyScienceModeRehastimGeneric:
        """Convert to a pyScienceMode device."""

    def initialize_stimulation(self) -> None:
        """Initialize the stimulation and the channels."""

        self._initialize_stimulation()
        self._is_stimulation_initialized = True

    @abstractmethod
    def _initialize_stimulation(self, channels: list[Channel]) -> None:
        """Initialize the stimulation at device level."""

    @abstractmethod
    def _get_channels(self) -> list[Channel]:
        """Initialize the channels."""


class Rehastim2(RehastimGeneric):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, **kwargs)
        self._channels: list[Channel] = [self._default_channel(channel_index=i + 1) for i in range(self.nb_channels)]
        self._channels_has_changed = True

    @override
    @property
    def device_name(self) -> str:
        return "Rehastim2"

    @override
    @property
    def nb_channels(self) -> int:
        return 8

    @abstractmethod
    def _default_channel(channel_index: int) -> Channel:
        """Get the default channel that will be used for the stimulation. This is used to initialize the stimulation, but
        real value can be changed by calling [set_amplitude].

        Parameters
        ----------
        channel_index : int
            The index of the channel. This is used to set the [no_channel] parameter of the channel.
        """

    @abstractproperty
    def _default_stimulation_interval(self) -> float:
        """Get the default stimulation interval that will be used for the stimulation. This is used to initialize the
        stimulation, but the real value can be changed by calling [set_interval]."""

    @abstractproperty
    def _default_low_frequency_factor(self) -> float:
        """Get the low frequency factor that will be used for the stimulation. This is used to initialize the
        stimulation, but the real value can be changed by calling [set_low_frequency_factor]."""

    @override
    def _get_initialized_device(self) -> pyScienceModeRehastimGeneric:
        from pyScienceMode.devices.rehastim2 import Rehastim2 as pyScienceModeRehastim2

        return pyScienceModeRehastim2(port=self.port, show_log=self.show_log)

    @override
    def _initialize_stimulation(self) -> None:
        from pyScienceMode.devices.rehastim2 import Rehastim2 as pyScienceModeRehastim2

        self._device: pyScienceModeRehastim2
        self._device.init_channel(
            stimulation_interval=self._default_stimulation_interval,
            list_channels=self._channels,
            low_frequency_factor=self._default_low_frequency_factor,
        )
        self._channels_has_changed = False

    def set_pulse_amplitude(self, amplitudes: float | list[float]) -> None:
        if isinstance(amplitudes, (float, int)):
            amplitudes = [amplitudes] * self.nb_channels

        for channel in self._channels:
            channel.set_amplitude(amplitudes[channel.get_no_channel() - 1])
        self._channels_has_changed = True

    def get_pulse_amplitude(self) -> list[float]:
        return [channel.get_amplitude() for channel in self._channels]

    def set_pulse_width(self, widths: int | list[int]) -> None:
        if isinstance(widths, int):
            widths = [widths] * self.nb_channels

        for channel in self._channels:
            channel.set_pulse_width(widths[channel.get_no_channel() - 1])
        self._channels_has_changed = True

    def get_pulse_width(self) -> list[int]:
        return [channel.get_pulse_width() for channel in self._channels]

    def set_pulse_interval(self, _: float) -> None:
        raise NotImplementedError(
            "The Rehastim2Device does not support changing the interval parameter once it is initialized."
        )

    def set_low_frequency_factor(self, _: float) -> None:
        raise NotImplementedError(
            "The Rehastim2Device does not support changing the low_frequency_factor parameter once it is initialized."
        )

    @override
    def reset_pulse_values_to_default(self) -> None:
        """Reset the stimulation to the default values."""
        self.set_pulse_amplitude(self._default_channel(channel_index=1).get_amplitude())
        self.set_pulse_interval(self._default_stimulation_interval)
        self.set_low_frequency_factor(self._default_low_frequency_factor)

    @override
    def _get_channel_list_for_stimulation(self) -> list[Channel] | None:
        if not self._is_stimulation_initialized:
            # On the very first call, we initialize the stimulation. This takes care of the channels at the same time.
            self.initialize_stimulation()

        channels = None
        if self._channels_has_changed:
            channels = self._get_channels()
        return channels

    @override
    def _get_channels(self) -> list[Channel]:
        return self._channels


class RehastimP24(RehastimGeneric):
    def __init__(self, port: str, show_log: bool = False) -> None:
        raise NotImplementedError("The RehastimP24Device is not implemented yet.")

    @override
    @property
    def device_name(self) -> str:
        return "RehastimP24"

    @override
    @property
    def nb_channels(self) -> int:
        return 24  # TODO: Check this

    @override
    def _get_initialized_device(self) -> pyScienceModeRehastimGeneric:
        from pyScienceMode.devices.rehastimP24 import RehastimP24 as pyScienceModeRehastimP24

        return pyScienceModeRehastimP24(port=self.port, show_log=self.show_log)

    @override
    def _initialize_stimulation(self) -> None:
        from pyScienceMode.devices.rehastimP24 import RehastimP24 as pyScienceModeRehastimP24

        self._device: pyScienceModeRehastimP24
        self._device.init_stimulation(list_channels=self.get_channels())

    @override
    def _get_channel_list_for_stimulation(self, duration: float = None) -> list[Channel] | None:
        raise NotImplementedError("The RehastimP24Device is not implemented yet.")

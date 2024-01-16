from .lokomat_rehastim import LokomatRehastim
from .pysciencemode_interfaces import Channel
from .rehastim_interface import RehastimDeviceAbstract


class RehastimDeviceMock(RehastimDeviceAbstract):
    @staticmethod
    def get_name():
        return "rehastim_mock"

    def _to_sciencemode(self):
        return self

    def _init_channel(self, channels: list[Channel], low_frequency_factor: int):
        pass


class ChannelMock(Channel):
    def __init__(self, channel: Channel) -> None:
        super(ChannelMock, self).__init__(
            mode=channel.mode,
            no_channel=channel.no_channel,
            amplitude=channel.amplitude,
            pulse_width=channel.pulse_width,
            enable_low_frequency=channel.enable_low_frequency,
            name=channel.name,
            device_type=channel.device_type,
            frequency=channel.frequency,
            ramp=channel.ramp,
        )

    def to_sciencemode(self):
        return self


class LokomatRehastimMock(LokomatRehastim):
    @property
    def _chosen_device(self):
        return RehastimDeviceMock

    @staticmethod
    def _initialize_channels(self) -> list[Channel]:
        channels = super(LokomatRehastimMock, self)._initialize_channels(self)
        return [ChannelMock(channel) for channel in channels]

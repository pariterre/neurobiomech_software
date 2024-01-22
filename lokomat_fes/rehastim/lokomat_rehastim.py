from pyScienceMode import Channel, Modes

from .rehastim_interface import Rehastim2Device


class LokomatRehastim(Rehastim2Device):
    def __init__(self, *args, **kwargs):
        super().__init__(stimulation_interval=200, low_frequency_factor=2, *args, **kwargs)

        self._channels = []
        for i in range(1, 9):
            self._channels.append(
                Channel(mode=Modes.SINGLE, no_channel=i, amplitude=50, pulse_width=100, device_type=self.device_name)
            )

    def _set_channels(self, channels: list[Channel]) -> None:
        raise RuntimeError("Channels for LokomatRehastim cannot be set.")

    def get_channels(self) -> list[Channel]:
        return self._channels

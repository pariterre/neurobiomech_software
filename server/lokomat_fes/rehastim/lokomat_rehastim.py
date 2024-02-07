from typing import override

from pyScienceMode import Channel, Modes

from .devices import Rehastim2


class RehastimLokomat(Rehastim2):
    @override
    def _default_channel(self, channel_index: int) -> Channel:
        return Channel(
            mode=Modes.SINGLE, no_channel=channel_index, amplitude=50, pulse_width=100, device_type=self.device_name
        )

    @override
    @property
    def _default_stimulation_interval(self) -> float:
        return 200

    @override
    @property
    def _default_low_frequency_factor(self) -> float:
        return 2

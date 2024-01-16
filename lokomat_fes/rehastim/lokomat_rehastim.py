from .pysciencemode_interfaces import Channel, Modes
from .rehastim_interface import RehastimInterface, Rehastim2Device


class LokomatRehastim(RehastimInterface):
    def __init__(self):
        channels = []
        device_type = self.device_name
        for i in range(8):
            channels.append(
                Channel(mode=Modes.SINGLE, no_channel=i, amplitude=50, pulse_width=100, device_type=device_type)
            )

        super().__init__(channels=channels, port="/dev/ttyUSB0", low_frequency_factor=2)

    @property
    def _chosen_device(self):
        return Rehastim2Device

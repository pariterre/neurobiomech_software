from enum import Enum


class Modes(Enum):
    """
    Class representing the different modes of the Rehastim2 stimulator emulating the pyScienceMode Modes class.
    """

    SINGLE = 0
    DOUBLET = 1
    TRIPLET = 2
    NONE = 3


class Channel:
    """
    Class representing a channel that emulates the pyScienceMode Channel class.
    """

    def __init__(
        self,
        mode: Modes = None,
        no_channel: int = 1,
        amplitude: float = 0,
        pulse_width: int = 0,
        enable_low_frequency: bool = False,
        name: str = None,
        device_type: str = None,
        frequency: float = 50.0,
        ramp: int = 0,
    ):
        self.mode = mode
        self.no_channel = no_channel
        self.amplitude = amplitude
        self.pulse_width = pulse_width
        self.enable_low_frequency = enable_low_frequency
        self.name = name
        self.device_type = device_type
        self.frequency = frequency
        self.ramp = ramp

    def to_sciencemode(self):
        from pyScienceMode import Channel as SciencemodeChannel

        return SciencemodeChannel(
            mode=self.mode,
            no_channel=self.no_channel,
            amplitude=self.amplitude,
            pulse_width=self.pulse_width,
            enable_low_frequency=self.enable_low_frequency,
            name=self.name,
            device_type=self.device_type,
            frequency=self.frequency,
            ramp=self.ramp,
        )

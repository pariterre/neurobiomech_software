from .devices import NiDaqGeneric


class NiDaqLokomat(NiDaqGeneric):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, num_channels=25, frame_rate=1000, **kwargs)

    def _channel_name(self, channel: int) -> str:
        return f"cDAQ1Mod1/ai{channel}"

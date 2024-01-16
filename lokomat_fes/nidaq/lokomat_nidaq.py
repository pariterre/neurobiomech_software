from .generic_nidaq import GenericNiDaq


class LokomatNiDaq(GenericNiDaq):
    def __init__(self, *args, **kwargs) -> None:
        super().__init__(*args, num_channels=25, frame_rate=1000, **kwargs)

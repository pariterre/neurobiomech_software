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


class LokomatRehastimMock(LokomatRehastim):
    @property
    def _chosen_device(self):
        return RehastimDeviceMock

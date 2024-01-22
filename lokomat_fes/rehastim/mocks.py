from pyScienceMode.devices.rehastim2 import Rehastim2
from pyScienceMode.devices.rehastim_generic import RehastimGeneric
from pyScienceMode import Rehastim2Commands

from .lokomat_rehastim import LokomatRehastim


class PortMock:
    def __init__(self, port: str):
        self.port = port

    def write(self, command: bytes):
        pass

    def close(self):
        pass


class Rehastim2Mock(Rehastim2):
    def _get_last_device_ack(self) -> list:
        return [None, None, None, None, None, 0]

    def _setup_device(self) -> None:
        self.port = PortMock(port=self.port_name)

    @property
    def are_channels_initialized(self):
        return self._are_channels_initialized


class LokomatRehastimMock(LokomatRehastim):
    def __init__(self, *args, **kwargs):
        super().__init__(port="NoPort", *args, **kwargs)

    def _get_initialized_device(self) -> RehastimGeneric:
        return Rehastim2Mock(port=self.port)

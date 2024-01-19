import pytest
import re

from lokomat_fes.rehastim import LokomatRehastimMock


def test_initialize():
    rehastim = LokomatRehastimMock()
    assert rehastim._device is not None
    assert rehastim._device.show_log is False
    assert rehastim._device.port == "/dev/ttyUSB0"
    assert rehastim._device.get_name() == "Rehastim2"
    rehastim.dispose()


def test_device_communication():
    rehastim = LokomatRehastimMock()
    rehastim.start_device()

    rehastim.start_stimulation(duration=0.1)
    # Since we are using multiprocessing, we are not able to check the state of the device.
    # So if no error is raised, we assume that the device is working.

    rehastim.dispose()


def test_starting_twice():
    rehastim = LokomatRehastimMock()
    rehastim.start_device()
    with pytest.raises(RuntimeError, match="The device is already started."):
        rehastim.start_device()
    rehastim.dispose()


def test_start_stimulation_without_starting():
    rehastim = LokomatRehastimMock()
    with pytest.raises(RuntimeError, match=re.escape("The device is not running, please call [start_device] before.")):
        rehastim.start_stimulation(duration=0.1)
    rehastim.dispose()

import time

from lokomat_fes.rehastim import LokomatRehastimMock


def test_initialize():
    rehastim = LokomatRehastimMock()
    assert rehastim._device is not None
    assert rehastim._device.show_log is False
    assert rehastim._device.port == "/dev/ttyUSB0"
    assert rehastim._device.get_name() == "Rehastim2"
    rehastim.dispose()


def test_send_stimulation():
    rehastim = LokomatRehastimMock()
    rehastim.perform_stimulation(duration=0.1)
    assert rehastim._device._pysciencemode_device._is_stimulating
    time.sleep(0.2)  # Wait for the stimulation to finish.
    assert not rehastim._device._pysciencemode_device._is_stimulating
    rehastim.dispose()

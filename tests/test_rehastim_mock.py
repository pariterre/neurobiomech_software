from lokomat_fes.rehastim import LokomatRehastimMock


def test_rehastim_initialize():
    rehastim = LokomatRehastimMock()
    assert rehastim._device is not None
    assert rehastim._device.show_log is False
    assert rehastim._device.port == "/dev/ttyUSB0"
    assert rehastim._device.get_name() == "Rehastim2"
    rehastim.dispose()

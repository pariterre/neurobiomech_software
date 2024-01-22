from lokomat_fes.rehastim.mocks import LokomatRehastimMock, Rehastim2Mock


def test_initialize():
    rehastim = LokomatRehastimMock()

    assert rehastim.show_log is False
    assert rehastim.port == "NoPort"
    assert rehastim.device_name == "Rehastim2"

    device: Rehastim2Mock = rehastim._device
    assert device is not None
    assert device.port.port == "NoPort"

    assert device.list_channels is None
    rehastim.initialize_stimulation()
    assert len(device.list_channels) == 8

    rehastim.dispose()


def test_initialize_by_starting_stimulation():
    rehastim = LokomatRehastimMock()

    device: Rehastim2Mock = rehastim._device
    assert device.list_channels is None
    rehastim.start_stimulation()
    assert len(device.list_channels) == 8

    rehastim.dispose()


def test_stop_stimulation():
    rehastim = LokomatRehastimMock()

    device: Rehastim2Mock = rehastim._device
    assert device.stimulation_active
    assert device.amplitude == []

    rehastim.start_stimulation()
    for amplitude in device.amplitude:
        assert amplitude == 50

    rehastim.stop_stimulation()

    rehastim.dispose()
    assert not device.stimulation_active

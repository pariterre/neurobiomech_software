import time

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


def test_stimulate_for_a_specific_duration():
    rehastim = LokomatRehastimMock()

    device: Rehastim2Mock = rehastim._device
    assert device.stimulation_active
    assert device.amplitude == []

    # Preinitialize so it doesn't mess up the timing
    rehastim.initialize_stimulation()

    # Both of these should be non-blocking
    initial_time = time.perf_counter()
    rehastim.start_stimulation()
    assert time.perf_counter() - initial_time < 0.2

    initial_time = time.perf_counter()
    rehastim.start_stimulation(duration=2)
    assert time.perf_counter() - initial_time < 0.2

    rehastim.dispose()
    assert not device.stimulation_active


def test_resuming_stimulation():
    rehastim = LokomatRehastimMock()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()


def test_stop_twice():
    rehastim = LokomatRehastimMock()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()

import time

from lokomat_fes.rehastim.mocks import RehastimLokomatMock, pyScienceModeRehastim2Mock


def test_initialize():
    rehastim = RehastimLokomatMock()

    assert rehastim.show_log is False
    assert rehastim.port == "NoPort"
    assert rehastim.device_name == "Rehastim2"

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device is not None
    assert device.port.port == "NoPort"

    assert device.list_channels is None
    rehastim.initialize_stimulation()
    assert len(device.list_channels) == 8

    rehastim.dispose()


def test_initialize_by_starting_stimulation():
    rehastim = RehastimLokomatMock()

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.list_channels is None
    rehastim.start_stimulation()
    assert len(device.list_channels) == 8

    rehastim.dispose()


def test_stop_stimulation():
    rehastim = RehastimLokomatMock()

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.stimulation_active
    assert device.amplitude == []

    rehastim.start_stimulation()
    for amplitude in device.amplitude:
        assert amplitude == 50

    rehastim.stop_stimulation()

    rehastim.dispose()
    assert not device.stimulation_active


def test_stimulating_with_callback():
    _callback_called = False

    def stimulation_callback(duration, channels):
        nonlocal _callback_called
        assert (channels is None) if _callback_called else (channels is not None)
        _callback_called = True

    rehastim = RehastimLokomatMock()
    rehastim.register_to_on_stimulation_started(stimulation_callback)

    rehastim.start_stimulation()
    assert _callback_called
    rehastim.start_stimulation()  # Second time, channels should be empty (no change)

    _callback_called = False
    rehastim.unregister_to_on_stimulation_started(stimulation_callback)
    rehastim.start_stimulation()
    assert not _callback_called

    rehastim.dispose()


def test_stimulate_for_a_specific_duration():
    rehastim = RehastimLokomatMock()

    device: pyScienceModeRehastim2Mock = rehastim._device
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
    rehastim = RehastimLokomatMock()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()


def test_stop_twice():
    rehastim = RehastimLokomatMock()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()

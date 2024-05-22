import pytest
import time

from lokomat_fes.rehastim.mocks import RehastimLokomatMock, pyScienceModeRehastim2Mock


def test_initialize():
    rehastim = RehastimLokomatMock(port="NoPort")

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
    rehastim = RehastimLokomatMock(port="NoPort")

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.list_channels is None
    rehastim.start_stimulation()
    assert len(device.list_channels) == 8

    rehastim.dispose()


def test_stop_stimulation():
    rehastim = RehastimLokomatMock(port="NoPort")

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

    def stimulation_callback(current_time, duration, channels):
        nonlocal _callback_called
        _callback_called = True
        assert channels is not None

    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.register_to_on_stimulation_changed(stimulation_callback)

    rehastim.start_stimulation()
    assert _callback_called
    rehastim.start_stimulation()  # Second time, channels should not be empty again

    _callback_called = False
    rehastim.unregister_to_on_stimulation_changed(stimulation_callback)
    rehastim.start_stimulation()
    assert not _callback_called

    rehastim.dispose()


def test_stimulate_for_a_specific_duration():
    rehastim = RehastimLokomatMock(port="NoPort")

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
    rehastim = RehastimLokomatMock(port="NoPort")

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.start_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()


def test_stop_twice():
    rehastim = RehastimLokomatMock(port="NoPort")

    rehastim.start_stimulation()
    rehastim.stop_stimulation()
    rehastim.stop_stimulation()

    rehastim.dispose()


def test_changing_pulse_amplitude():
    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.initialize_stimulation()

    assert rehastim.get_pulse_amplitude() == [50] * 8  # Default amplitude

    rehastim.set_pulse_amplitude(100)
    assert rehastim.get_pulse_amplitude() == [100] * 8

    rehastim.dispose()


def test_changing_pulse_width():
    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.initialize_stimulation()

    assert rehastim.get_pulse_width() == [100] * 8  # Default width

    rehastim.set_pulse_width(200)
    assert rehastim.get_pulse_width() == [200] * 8

    rehastim.dispose()


def test_changing_pulse_interval():
    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.initialize_stimulation()

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.stimulation_interval == 200  # Default interval

    with pytest.raises(
        NotImplementedError,
        match="The Rehastim2Device does not support changing the interval parameter once it is initialized.",
    ):
        rehastim.set_pulse_interval(0)

    rehastim.dispose()


def test_changing_low_frequency_factor():
    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.initialize_stimulation()

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.low_frequency_factor == 2  # Default low frequency factor

    with pytest.raises(
        NotImplementedError,
        match="The Rehastim2Device does not support changing the low_frequency_factor parameter once it is initialized.",
    ):
        rehastim.set_low_frequency_factor(0.5)

    rehastim.dispose()


def test_reset_pulse_to_default():
    rehastim = RehastimLokomatMock(port="NoPort")
    rehastim.initialize_stimulation()

    device: pyScienceModeRehastim2Mock = rehastim._device
    assert device.amplitude == [50] * 8  # Default amplitude
    assert device.pulse_width == [100] * 8  # Default width
    assert device.stimulation_interval == 200  # Default interval

    rehastim.set_pulse_amplitude(100)
    rehastim.set_pulse_width(200)
    rehastim.start_stimulation()  # Starting the stimulation is needed to actually change the pulse width for the device
    rehastim.stop_stimulation()
    assert device.amplitude == [100] * 8
    assert device.pulse_width == [200] * 8

    with pytest.raises(
        NotImplementedError,
        match="The Rehastim2Device does not support changing the interval parameter once it is initialized.",
    ):
        rehastim.reset_pulse_values_to_default()

    rehastim.dispose()

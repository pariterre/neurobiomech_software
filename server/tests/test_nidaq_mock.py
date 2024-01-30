import pytest
import time

from lokomat_fes.nidaq.mocks import NiDaqLokomatMock


def test_nidaq_initialize():
    nidaq = NiDaqLokomatMock()
    assert nidaq.num_channels == 25
    assert nidaq.frame_rate == 1000
    assert nidaq.dt == 1 / 1000
    nidaq.dispose()


def test_start_recording():
    nidaq = NiDaqLokomatMock()
    nidaq.start_recording()
    assert nidaq._is_recording
    assert nidaq._timer is not None
    nidaq.dispose()


def test_start_recording_with_callback():
    _callback_called = False

    def start_recording_callback():
        nonlocal _callback_called
        _callback_called = True

    nidaq = NiDaqLokomatMock()
    nidaq.register_to_start_recording(start_recording_callback)
    nidaq.start_recording()
    assert nidaq._is_recording
    assert nidaq._timer is not None
    assert _callback_called
    nidaq.stop_recording()

    _callback_called = False
    nidaq.unregister_to_start_recording(start_recording_callback)
    nidaq.start_recording()
    nidaq.stop_recording()
    assert not _callback_called

    nidaq.dispose()


def test_stop_recording():
    nidaq = NiDaqLokomatMock()
    nidaq.start_recording()
    nidaq.stop_recording()
    assert not nidaq._is_recording
    assert nidaq._timer is None
    nidaq.dispose()


def test_stop_recording_with_callback():
    _callback_called = False

    def stop_recording_callback(data):
        nonlocal _callback_called
        _callback_called = True

    nidaq = NiDaqLokomatMock()
    nidaq.register_to_stop_recording(stop_recording_callback)
    nidaq.start_recording()
    nidaq.stop_recording()
    assert not nidaq._is_recording
    assert nidaq._timer is None
    assert _callback_called

    _callback_called = False
    nidaq.unregister_to_stop_recording(stop_recording_callback)
    nidaq.start_recording()
    nidaq.stop_recording()
    assert not _callback_called

    nidaq.dispose()


def test_start_recording_twice():
    nidaq = NiDaqLokomatMock()
    nidaq.start_recording()
    with pytest.raises(RuntimeError, match="Already recording"):
        nidaq.start_recording()
    nidaq.dispose()


def test_generate_fake_data():
    nidaq = NiDaqLokomatMock()

    nidaq._generate_fake_data()
    assert len(nidaq._data._t) == 1
    assert nidaq._data._t[0].shape == (1000,)
    assert len(nidaq._data._data) == 1
    assert nidaq._data._data[0].shape == (25, 1000)

    nidaq._generate_fake_data()
    assert len(nidaq._data._t) == 2
    assert nidaq._data._t[1].shape == (1000,)
    assert len(nidaq._data._data) == 2
    assert nidaq._data._data[1].shape == (25, 1000)

    nidaq.dispose()


def test_on_data_ready_callback():
    _callback_called = False

    def data_ready_callback(t, samples):
        nonlocal _callback_called
        _callback_called = True

    nidaq = NiDaqLokomatMock()
    nidaq.register_to_data_ready(data_ready_callback)
    nidaq._generate_fake_data()
    assert len(nidaq._data._t) == 1
    assert len(nidaq._data._data) == 1
    assert _callback_called

    _callback_called = False
    nidaq.unregister_to_data_ready(data_ready_callback)
    nidaq._generate_fake_data()
    assert not _callback_called

    nidaq.dispose()


def test_start_recording_records_data():
    _callback_called = 0

    def data_ready_callback(t, samples):
        nonlocal _callback_called
        _callback_called += 1

    nidaq = NiDaqLokomatMock()
    nidaq.register_to_data_ready(data_ready_callback)
    nidaq.start_recording()
    time.sleep(2 * nidaq._time_between_samples)  # Wait for long enough so we get at least 1 sample
    nidaq.stop_recording()

    assert _callback_called >= 1  # Do not use == as the timer might not be precise

    nidaq.dispose()


def test_resuming_recording():
    nidaq = NiDaqLokomatMock()

    nidaq.start_recording()
    nidaq.stop_recording()

    nidaq.start_recording()
    nidaq.stop_recording()

    nidaq.dispose()

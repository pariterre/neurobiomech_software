import os
import time
import pytest

from lokomat_fes.rehastim.data import RehastimData, Channel
import numpy as np
from pyScienceMode import Channel as pyScienceModeChannel, Modes, Device


def _generate_data(rehastim_data: RehastimData):
    rehastim_data.add(
        duration=2,
        channels=(
            pyScienceModeChannel(mode=Modes.SINGLE, no_channel=1, amplitude=2, device_type=Device.Rehastim2),
            pyScienceModeChannel(mode=Modes.SINGLE, no_channel=2, amplitude=4, device_type=Device.Rehastim2),
        ),
    )
    rehastim_data.add(duration=4, channels=None)
    rehastim_data.add(
        duration=6,
        channels=(
            Channel(channel_index=1, amplitude=8),
            Channel(channel_index=2, amplitude=10),
        ),
    )


def test_data_creation():
    rehastim_data = RehastimData()
    assert rehastim_data._data == []
    assert rehastim_data.time.shape == (0,)
    assert rehastim_data.duration_as_array.shape == (1, 0)
    assert rehastim_data.amplitude_as_array.shape == (1, 0)


def test_len_data():
    rehastim_data = RehastimData()

    # Check that the data is correct
    assert len(rehastim_data) == 0

    # Add data
    _generate_data(rehastim_data)

    # Check that the data is correct
    assert len(rehastim_data) == 3


def test_has_data():
    rehastim_data = RehastimData()

    # Check that the data is correct
    assert not rehastim_data.has_data

    # Add data
    _generate_data(rehastim_data)

    # Check that the data is correct
    assert rehastim_data.has_data


def test_data_time():
    rehastim_data = RehastimData()

    # Add data
    now = time.time()
    time.sleep(0.01)  # Make sure that the time is different
    rehastim_data.add(
        duration=2,
        channels=(
            pyScienceModeChannel(mode=Modes.SINGLE, no_channel=1, amplitude=2, device_type=Device.Rehastim2),
            pyScienceModeChannel(mode=Modes.SINGLE, no_channel=2, amplitude=4, device_type=Device.Rehastim2),
        ),
    )
    time.sleep(0.01)  # Make sure that the time is different
    then = time.time()

    # Get the data back
    t, _, _ = rehastim_data.sample_block(index=0)

    # Check that the data is correct
    assert t.timestamp() >= now
    assert t.timestamp() <= then


def test_must_have_channels_on_first_call():
    rehastim_data = RehastimData()

    # Add data
    with pytest.raises(RuntimeError, match="The first time you add data, you must specify the channels."):
        rehastim_data.add(duration=2, channels=None)


def test_time_accessor():
    rehastim_data = RehastimData()

    # Add data
    now = time.time()
    _generate_data(rehastim_data)
    then = time.time() - now
    time.sleep(0.01)  # Make sure that the time is different
    _generate_data(rehastim_data)

    # Get the data back by index
    t = rehastim_data.time
    assert t.shape == (6,)
    assert t[0] >= 0
    assert t[0] <= then
    assert t[-1] >= then


def test_duration_accessor():
    rehastim_data = RehastimData()

    # Add data
    _generate_data(rehastim_data)

    # Get the data back by index
    duration = rehastim_data.duration_as_array
    assert duration.shape == (3,)
    np.testing.assert_almost_equal(duration, [2, 4, 6])


def test_amplitude_accessor():
    rehastim_data = RehastimData()

    # Add data
    _generate_data(rehastim_data)

    # Get the data back by index
    amplitude = rehastim_data.amplitude_as_array
    assert amplitude.shape == (2, 3)
    np.testing.assert_almost_equal(amplitude, [[2, 2, 8], [4, 4, 10]])


def test_block_data_accessor():
    rehastim_data = RehastimData()

    # Add data
    _generate_data(rehastim_data)

    # Get the data back by index
    _, duration, channels = rehastim_data.sample_block(index=1)
    assert duration == 4
    assert [channel.amplitude for channel in channels] == [2, 4]

    # Get the data back asking for last
    _, duration, channels = rehastim_data.sample_block(index=-1)
    assert duration == 6
    assert [channel.amplitude for channel in channels] == [8, 10]

    # Get the data by a slice
    data = rehastim_data.sample_block(index=slice(1, 3))
    assert len(data) == 2
    _, duration, channels = data[0]
    assert duration == 4
    assert [channel.amplitude for channel in channels] == [2, 4]
    _, duration, channels = data[1]
    assert duration == 6
    assert [channel.amplitude for channel in channels] == [8, 10]


def test_copy_rehastim_data():
    rehastim_data = RehastimData()

    # Add data
    _generate_data(rehastim_data)

    # Copy the data
    rehastim_data_copy = rehastim_data.copy

    # Check that the data is correct
    assert rehastim_data_copy.time.shape == rehastim_data.time.shape
    assert rehastim_data_copy.duration_as_array.shape == rehastim_data.duration_as_array.shape
    assert rehastim_data_copy.amplitude_as_array.shape == rehastim_data.amplitude_as_array.shape
    np.testing.assert_almost_equal(rehastim_data_copy.time, rehastim_data.time)
    np.testing.assert_almost_equal(rehastim_data_copy.duration_as_array, rehastim_data.duration_as_array)
    np.testing.assert_almost_equal(rehastim_data_copy.amplitude_as_array, rehastim_data.amplitude_as_array)

    # The copy is a deep copy
    rehastim_data_copy._data[1] = (0, 0, 0)
    np.testing.assert_almost_equal(rehastim_data_copy._data[1][1:][0], 0)
    np.testing.assert_almost_equal(rehastim_data._data[1][1:][0], 4)


def test_save_and_load():
    rehastim_data = RehastimData()

    # Add data
    _generate_data(rehastim_data)

    # Save and load
    rehastim_data.save("data.pkl")
    rehastim_data_loaded = RehastimData.load("data.pkl")
    os.remove("data.pkl")

    # Check that the data is correct
    assert rehastim_data_loaded.time.shape == rehastim_data.time.shape
    assert rehastim_data_loaded.duration_as_array.shape == rehastim_data.duration_as_array.shape
    assert rehastim_data_loaded.amplitude_as_array.shape == rehastim_data.amplitude_as_array.shape
    np.testing.assert_almost_equal(rehastim_data_loaded.time, rehastim_data.time)
    np.testing.assert_almost_equal(rehastim_data_loaded.duration_as_array, rehastim_data.duration_as_array)
    np.testing.assert_almost_equal(rehastim_data_loaded.amplitude_as_array, rehastim_data.amplitude_as_array)

    # The copy is a deep copy
    rehastim_data_loaded._data[1] = (0, 0, 0)
    np.testing.assert_almost_equal(rehastim_data_loaded._data[1][1:][0], 0)
    np.testing.assert_almost_equal(rehastim_data._data[1][1:][0], 4)

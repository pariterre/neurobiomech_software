import os
import time

import numpy as np

from lokomat_fes.rehastim import RehastimData


def test_data_creation():
    rehastim_data = RehastimData()
    assert rehastim_data._data == []
    assert rehastim_data.time.shape == (0,)
    assert rehastim_data.duration_as_array.shape == (1, 0)
    assert rehastim_data.amplitude_as_array.shape == (1, 0)


def test_data_add_data():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    now = time.time()
    rehastim_data.add(4, 5)
    then = time.time()
    rehastim_data.add(6, 7)

    # Get the data back
    t, duration, amplitude = rehastim_data.sample_block(index=1)

    # Check that the data is correct
    assert t.timestamp() >= now
    assert t.timestamp() <= then
    assert duration == 4
    assert amplitude == 5


def test_time_accessor():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

    # Get the data back by index
    t = rehastim_data.time
    assert t.shape == (3,)
    np.testing.assert_almost_equal(t[0], 0)


def test_duration_accessor():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

    # Get the data back by index
    duration = rehastim_data.duration_as_array
    assert duration.shape == (3,)
    np.testing.assert_almost_equal(duration, [2, 4, 6])


def test_amplitude_accessor():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

    # Get the data back by index
    amplitude = rehastim_data.amplitude_as_array
    assert amplitude.shape == (3,)
    np.testing.assert_almost_equal(amplitude, [3, 5, 7])


def test_block_data_accessor():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

    # Get the data back by index
    _, duration, amplitude = rehastim_data.sample_block(index=1)
    assert duration == 4
    assert amplitude == 5

    # Get the data back asking for last
    _, duration, amplitude = rehastim_data.sample_block(index=-1)
    assert duration == 6
    assert amplitude == 7

    # Get the data by a slice
    _, duration, amplitude = rehastim_data.sample_block(index=slice(1, 3))
    assert duration == [4, 6]
    assert amplitude == [5, 7]


def test_copy_rehastim_data():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

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
    np.testing.assert_almost_equal(rehastim_data_copy._data[1][1:], (0, 0))
    np.testing.assert_almost_equal(rehastim_data._data[1][1:], (4, 5))


def test_save_and_load():
    rehastim_data = RehastimData()

    # Add data
    rehastim_data.add(2, 3)
    rehastim_data.add(4, 5)
    rehastim_data.add(6, 7)

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
    np.testing.assert_almost_equal(rehastim_data_loaded._data[1][1:], (0, 0))
    np.testing.assert_almost_equal(rehastim_data._data[1][1:], (4, 5))

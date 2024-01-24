import os
import time

import numpy as np

from lokomat_fes.nidaq import NiDaqData


def test_data_creation():
    nidaq_data = NiDaqData()
    assert nidaq_data.start_recording_time is None
    assert nidaq_data.time.shape == (0,)
    assert nidaq_data.as_array.shape == (1, 0)


def test_data_add_data():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Add data
    now = time.time()
    # The first call set the start recording time
    time.sleep(0.01)  # Make sure that the time is different
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    time.sleep(0.01)  # Make sure that the time is different
    then = time.time()
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Get the data back
    start_recording_time = nidaq_data.start_recording_time
    t_data = nidaq_data.time
    data = nidaq_data.as_array

    # Check that the data is correct
    assert start_recording_time.timestamp() >= now
    assert start_recording_time.timestamp() <= then
    assert t_data.shape == (n_frames * 3,)
    assert data.shape == (1, n_frames * 3)


def test_has_data():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Check that the data is correct
    assert not nidaq_data.has_data

    # Add data
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Check that the data is correct
    assert nidaq_data.has_data


def test_block_data_accessor():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Add data
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Get the data back by index
    t_data, data = nidaq_data.sample_block(index=1)
    assert t_data.shape == (n_frames,)
    assert data.shape == (1, n_frames)
    np.testing.assert_almost_equal(t_data[0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(t_data[-1], t[-1] + block_time * 1)
    np.testing.assert_almost_equal(data[0, 0], np.sin(t[0] + block_time * 1))
    np.testing.assert_almost_equal(data[0, -1], np.sin(t[-1] + block_time * 1))

    # The default behavior for sample_block is to return a deep copy of the data
    t_data[0] = 0
    data[0, 0] = 0
    np.testing.assert_almost_equal(t_data[0], 0)
    np.testing.assert_almost_equal(data[0, 0], 0)
    np.testing.assert_almost_equal(nidaq_data._t[1][0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(nidaq_data._data[1][0, 0], np.sin(t[0] + block_time * 1))

    # Get the data back asking for last
    t_data, data = nidaq_data.sample_block(index=-1)
    assert t_data.shape == (n_frames,)
    assert data.shape == (1, n_frames)
    np.testing.assert_almost_equal(t_data[0], t[0] + block_time * 2)
    np.testing.assert_almost_equal(t_data[-1], t[-1] + block_time * 2)

    # Get the data by a slice
    t_data, data = nidaq_data.sample_block(index=slice(1, 3))
    assert len(t_data) == 2
    assert len(data) == 2
    assert t_data[0].shape == (n_frames,)
    assert data[0].shape == (1, n_frames)
    assert t_data[1].shape == (n_frames,)
    assert data[1].shape == (1, n_frames)
    np.testing.assert_almost_equal(t_data[0][0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(t_data[0][-1], t[0] + block_time * 2)
    np.testing.assert_almost_equal(t_data[1][0], t[0] + block_time * 2)
    np.testing.assert_almost_equal(t_data[1][-1], t[0] + block_time * 3)
    np.testing.assert_almost_equal(data[0][0, 0], np.sin(t[0] + block_time * 1))
    np.testing.assert_almost_equal(data[0][0, -1], np.sin(t[-1] + block_time * 1))
    np.testing.assert_almost_equal(data[1][0, 0], np.sin(t[0] + block_time * 2))
    np.testing.assert_almost_equal(data[1][0, -1], np.sin(t[-1] + block_time * 2))

    # Get the data via unsafe accessor
    t_data, data = nidaq_data.sample_block(index=1, unsafe=True)
    assert t_data.shape == (n_frames,)
    assert data.shape == (1, n_frames)
    np.testing.assert_almost_equal(t_data[0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(t_data[-1], t[-1] + block_time * 1)
    np.testing.assert_almost_equal(data[0, 0], np.sin(t[0] + block_time * 1))
    np.testing.assert_almost_equal(data[0, -1], np.sin(t[-1] + block_time * 1))

    # The unsafe accessor returns a reference to the original data
    t_data[0] = 0
    data[0, 0] = 0
    np.testing.assert_almost_equal(t_data[0], 0)
    np.testing.assert_almost_equal(data[0, 0], 0)
    np.testing.assert_almost_equal(nidaq_data._t[1][0], 0)
    np.testing.assert_almost_equal(nidaq_data._data[1][0, 0], 0)


def test_add_sample_block_data():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Add data
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Get the data back by index
    sample_block = nidaq_data.sample_block(index=1, unsafe=True)

    # Now create a new data object and add the sample block
    new_nidaq_data = NiDaqData()
    new_nidaq_data.add_sample_block(*sample_block)

    # This should now create a shallow copy of the data, so compare their id
    assert id(new_nidaq_data._t[0]) == id(nidaq_data._t[1])


def test_copy_nidaq_data():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Add data
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Copy the data
    nidaq_data_copy = nidaq_data.copy

    # Check that the data is correct
    assert nidaq_data_copy.start_recording_time == nidaq_data.start_recording_time
    assert nidaq_data_copy.time.shape == nidaq_data.time.shape
    assert nidaq_data_copy.as_array.shape == nidaq_data.as_array.shape
    np.testing.assert_almost_equal(nidaq_data_copy.time, nidaq_data.time)
    np.testing.assert_almost_equal(nidaq_data_copy.as_array, nidaq_data.as_array)

    # The copy is a deep copy
    nidaq_data_copy._t[1][0] = 0
    nidaq_data_copy._data[1][0, 0] = 0
    np.testing.assert_almost_equal(nidaq_data_copy._t[1][0], 0)
    np.testing.assert_almost_equal(nidaq_data_copy._data[1][0, 0], 0)
    np.testing.assert_almost_equal(nidaq_data._t[1][0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(nidaq_data._data[1][0, 0], np.sin(t[0] + block_time * 1))


def test_save_and_load():
    nidaq_data = NiDaqData()

    # Generate some fake data
    n_frames = 100
    block_time = 1
    t = np.linspace(0, block_time, n_frames)

    # Add data
    nidaq_data.add(t + block_time * 0, np.sin(t + block_time * 0)[np.newaxis, :])
    nidaq_data.add(t + block_time * 1, np.sin(t + block_time * 1)[np.newaxis, :])
    nidaq_data.add(t + block_time * 2, np.sin(t + block_time * 2)[np.newaxis, :])

    # Save and load
    nidaq_data.save("data.pkl")
    nidaq_data_loaded = NiDaqData.load("data.pkl")
    os.remove("data.pkl")

    # Check that the data is correct
    assert nidaq_data_loaded.start_recording_time == nidaq_data.start_recording_time
    assert nidaq_data_loaded.time.shape == nidaq_data.time.shape
    assert nidaq_data_loaded.as_array.shape == nidaq_data.as_array.shape
    np.testing.assert_almost_equal(nidaq_data_loaded.time, nidaq_data.time)
    np.testing.assert_almost_equal(nidaq_data_loaded.as_array, nidaq_data.as_array)

    # The copy is a deep copy
    nidaq_data_loaded._t[1][0] = 0
    nidaq_data_loaded._data[1][0, 0] = 0
    np.testing.assert_almost_equal(nidaq_data_loaded._t[1][0], 0)
    np.testing.assert_almost_equal(nidaq_data_loaded._data[1][0, 0], 0)
    np.testing.assert_almost_equal(nidaq_data._t[1][0], t[0] + block_time * 1)
    np.testing.assert_almost_equal(nidaq_data._data[1][0, 0], np.sin(t[0] + block_time * 1))

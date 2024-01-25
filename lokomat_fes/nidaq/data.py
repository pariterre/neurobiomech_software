from copy import deepcopy
from datetime import datetime
import pickle

import numpy as np


class NiDaqData:
    """Class to store data from NI DAQ devices.

    Attributes
    ----------
    _t0 : datetime | None
        Starting time of the recording (set at the moment of the declaration of the class or that the
        time [set_t0] is called).
    _t0_offset : float
        Offset to add to the time to get the real time (in seconds). It is the difference between the t0 and actual first data.
        This is to account for the fact that the NI DAQ device takes time to launch, but the first data are still
        considered to be at time=0. The offset makes sure the _t0 (in datetime) actually corresponds to the first data
        so it can be synchronized with other devices. This value should correspond to the first time of the first data block.
    _t : list[np.ndarray]
        List of time vectors.
    _data : list[np.ndarray]
        List of data vectors.
    """

    def __init__(self) -> None:
        self._t0: datetime = datetime.now()
        self._t0_offset: float = 0  # Offset to add to the time to get the real time. It is the difference between the t0 and actual first data received.
        self._t: list[np.ndarray] = []
        self._data: list[np.ndarray] = []

    def set_t0(self, new_t0: datetime | None) -> None:
        """Reset the starting time of the recording.

        Parameters
        ----------
        new_t0 : datetime | None
            New starting time of the recording. If None, the starting time is set to the current time.
        """
        self._t0 = new_t0 if new_t0 is not None else datetime.now()

    def _set_t0_offset(self) -> None:
        """Reset the starting time offset of the recording."""
        self._t0_offset = (datetime.now() - self._t0).microseconds / 1e6  # seconds

    def add(self, t: np.ndarray, data: np.ndarray) -> None:
        """Add data from a NI DAQ device to the data.
        Data are expected to be [channels x time].

        Parameters
        ----------
        t : np.ndarray
            Time vector of the new data
        data : np.ndarray
            Data vector
        """
        if not self.has_data:
            self._set_t0_offset()

        self._t.append(t + self._t0_offset)
        self._data.append(data)

    def add_sample_block(self, t: np.ndarray, data: np.ndarray) -> None:
        """Add data from a NI DAQ device to the data.
        Data are expected to from a single sample block. This creates a shallow copy of the data.

        Parameters
        ----------
        t : np.ndarray
            Time vector of the new data
        data : np.ndarray
            Data vector
        """
        if not self.has_data:
            self._set_t0_offset()

        self._t.append(t + self._t0_offset)
        self._data.append(data)

    @property
    def has_data(self) -> bool:
        """Check if the data contains data.

        Returns
        -------
        out : bool
            True if the data contains data, False otherwise.
        """
        return len(self._t) > 0

    def sample_block(self, index: int | slice, unsafe: bool = False) -> tuple[np.ndarray | None, np.ndarray | None]:
        """Get a block of data.

        Parameters
        ----------
        index : int | slice
            Index of the block.
        unsafe : bool
            If True, the data are returned as a reference to the original data (meaning it is faster but it can be
            modified by the caller). If False, the data are copied (meaning it is slower but data are garanteed to be
            preserved).

        Returns
        -------
        t : np.ndarray
            Time vector of the data. Warning, this is without the t0 offset!
        data : np.ndarray
            Data from the NI DAQ device.
        """
        if not self.has_data:
            return None, None

        if unsafe:
            return self._t[index], self._data[index]
        else:
            return deepcopy(self._t[index]), deepcopy(self._data[index])

    @property
    def t0(self) -> datetime:
        """Get the starting time of the recording (without t0 offset).

        Returns
        -------
        t : datetime
            Starting time of the recording.
        """
        return deepcopy(self._t0)

    @property
    def time(self) -> np.ndarray:
        """Get time from a NI DAQ device.

        Returns
        -------
        t : np.ndarray
            Time vector of the data.
        """
        if not self.has_data:
            return np.array([])

        return np.concatenate(self._t)

    @property
    def as_array(self) -> np.ndarray:
        """Get data to a numpy array.

        Parameters
        ----------

        Returns
        -------
        data : np.ndarray
            Data from the NI DAQ device.
        """
        if not self.has_data:
            return np.array([[]])

        return np.concatenate(self._data, axis=1)

    @property
    def copy(self) -> "NiDaqData":
        """Get a copy of the data.

        Returns
        -------
        out : NiDaqData
            Copy of the data.
        """

        out = NiDaqData()
        out._t0 = deepcopy(self._t0)
        out._t0_offset = deepcopy(self._t0_offset)
        out._t = deepcopy(self._t)
        out._data = deepcopy(self._data)
        return out

    def save(self, path: str) -> None:
        """Save the data to a file.

        Parameters
        ----------
        path : str
            Path to the file.
        """

        with open(path, "wb") as f:
            pickle.dump(self.serialized, f)

    @classmethod
    def load(cls, path: str) -> "NiDaqData":
        """Load the data from a file.

        Parameters
        ----------
        path : str
            Path to the file.

        Returns
        -------
        out : NiDaqData
            Loaded data.
        """

        with open(path, "rb") as f:
            data = pickle.load(f)
        return cls.deserialize(data)

    @property
    def serialized(self) -> dict:
        """Serialize the data.

        Returns
        -------
        out : dict
            Serialized data.
        """
        return {
            "t0": self._t0,
            "t0_offset": self._t0_offset,
            "t": self._t,
            "data": self._data,
        }

    @classmethod
    def deserialize(cls, data: dict) -> "NiDaqData":
        """Deserialize the data.

        Parameters
        ----------
        data : dict
            Serialized data.

        Returns
        -------
        out : NiDaqData
            Deserialized data.
        """
        out = cls()
        out._t0 = data["t0"]
        out._t0_offset = data["t0_offset"]
        out._t = data["t"]
        out._data = data["data"]
        return out

from copy import deepcopy
from datetime import datetime
import pickle

import numpy as np


class NiDaqData:
    """Class to store data from NI DAQ devices.

    Attributes
    ----------
    data : dict
        Dictionary with data from NI DAQ devices.
    """

    def __init__(self) -> None:
        self._start_recording_time: datetime | None = None
        self._t: list[np.ndarray] = []
        self._data: list[np.ndarray] = []

    def add(self, t, data) -> None:
        """Add data from a NI DAQ device to the data.
        Data are expected to be [channels x time].

        Parameters
        ----------
        t : np.ndarray
            Time vector of the new data
        data : np.ndarray
            Data vector
        """

        if self._start_recording_time is None:
            self._start_recording_time = datetime.now()
        self._t.append(t)
        self._data.append(data)

    def sample_block(
        self, index: int | slice | range, unsafe: bool = False
    ) -> tuple[np.ndarray | None, np.ndarray | None]:
        """Get a block of data.

        Parameters
        ----------
        index : int | slice | range
            Index of the block.
        unsafe : bool
            If True, the data are returned as a reference to the original data (meaning it is faster but it can be
            modified by the caller). If False, the data are copied (meaning it is slower but data are garanteed to be
            preserved).

        Returns
        -------
        t : np.ndarray
            Time vector of the data.
        data : np.ndarray
            Data from the NI DAQ device.
        """
        if not self._t:
            return None, None

        if unsafe:
            return self._t[index], self._data[index]
        else:
            return deepcopy(self._t[index]), deepcopy(self._data[index])

    @property
    def start_recording_time(self) -> datetime | None:
        """Get the starting time of the recording.

        Returns
        -------
        t : datetime
            Starting time of the recording.
        """
        return deepcopy(self._start_recording_time)

    @property
    def time(self) -> np.ndarray:
        """Get time from a NI DAQ device.

        Returns
        -------
        t : np.ndarray
            Time vector of the data.
        """
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
        out._t = deepcopy(self._t)
        out._data = deepcopy(self._data)
        out._start_recording_time = deepcopy(self._start_recording_time)
        return out

    def save(self, path: str) -> None:
        """Save the data to a file.

        Parameters
        ----------
        path : str
            Path to the file.
        """

        with open(path, "wb") as f:
            pickle.dump(self.serialize, f)

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
    def serialize(self) -> dict:
        """Serialize the data.

        Returns
        -------
        out : dict
            Serialized data.
        """
        return {
            "start_recording_time": self._start_recording_time,
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
        out._start_recording_time = data["start_recording_time"]
        out._t = data["t"]
        out._data = data["data"]
        return out

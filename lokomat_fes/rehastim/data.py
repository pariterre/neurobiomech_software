from copy import deepcopy
from datetime import datetime
import pickle

import numpy as np


class RehastimData:
    """Class to store data from Rehastim devices.

    Attributes
    ----------
    _data: list[tuple[datetime, float, float]]
        List of data vectors. Each vector is a tuple of (time [datetime], duration [float], amplitude [float]).
    """

    def __init__(self) -> None:
        self._data: list[tuple[datetime, float, float]] = []

    def add(self, duration: float, amplitude: float) -> None:
        """Add data from a Rehastim device to the data.

        Parameters
        ----------
        duration : float
            Duration of the stimulation.
        amplitude : float
            Amplitude of the stimulation.
        """

        self._data.append((datetime.now(), duration, amplitude))

    def sample_block(self, index: int | slice) -> tuple[datetime | None, float | None, float | None]:
        """Get a block of data.

        Parameters
        ----------
        index : int | slice
            Index of the block.

        Returns
        -------
        t : datetime
            Time of the data.
        duration : float
            Duration of the stimulation.
        amplitude : float
            Amplitude of the stimulation.
        """
        if not self._data:
            return None, None, None

        return self._data[index]

    @property
    def time(self) -> np.ndarray:
        """Get time of each event.

        Returns
        -------
        t : np.ndarray
            Time vector of the data.
        """
        if not self._data:
            return np.array([])

        t0 = self._data[0][0]
        return np.array([(t - t0).total_seconds() for t, _, _ in self._data])

    @property
    def duration_as_array(self) -> np.ndarray:
        """Get duration data to a numpy array.

        Parameters
        ----------

        Returns
        -------
        data : np.ndarray
            Duration of each stimulation
        """
        if not self._data:
            return np.array([[]])

        return np.array([d for _, d, _ in self._data])

    @property
    def amplitude_as_array(self) -> np.ndarray:
        """Get amplitude data to a numpy array.

        Parameters
        ----------

        Returns
        -------
        data : np.ndarray
            Amplitude of each stimulation
        """
        if not self._data:
            return np.array([[]])

        return np.array([a for _, _, a in self._data])

    @property
    def copy(self) -> "RehastimData":
        """Get a copy of the data.

        Returns
        -------
        out : RehastimData
            Copy of the data.
        """

        out = RehastimData()
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
            pickle.dump(self.serialize, f)

    @classmethod
    def load(cls, path: str) -> "RehastimData":
        """Load the data from a file.

        Parameters
        ----------
        path : str
            Path to the file.

        Returns
        -------
        out : RehastimData
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
        return {"data": self._data}

    @classmethod
    def deserialize(cls, data: dict) -> "RehastimData":
        """Deserialize the data.

        Parameters
        ----------
        data : dict
            Serialized data.

        Returns
        -------
        out : RehastimData
            Deserialized data.
        """
        out = cls()
        out._data = data["data"]
        return out

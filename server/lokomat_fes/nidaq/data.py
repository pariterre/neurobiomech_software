from copy import deepcopy
from datetime import datetime
import pickle
from typing import Any

import numpy as np
import matplotlib.pyplot as plt


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

    def __init__(
        self,
        t0: datetime | None = None,
        t0_offset: float | None = None,
        t: list[np.ndarray] | None = None,
        data: list[np.ndarray] | None = None,
    ) -> None:
        """
        Parameters
        ----------
        t0 : datetime | None
            Starting time of the recording (set at the moment of the declaration of the class or that the
            time [set_t0] is called).
        t0_offset : float | None
            Offset to add to the time to get the real time (in seconds). It is the difference between the t0 and actual first data.
            This is to account for the fact that the NI DAQ device takes time to launch, but the first data are still
            considered to be at time=0. The offset makes sure the _t0 (in datetime) actually corresponds to the first data
            so it can be synchronized with other devices. This value should correspond to the first time of the first data block.
        t : list[np.ndarray] | None
            List of time vectors.
        data : list[np.ndarray] | None
            List of data vectors.
        """
        self._t0: datetime = datetime.now() if t0 is None else t0
        self._t0_offset: float = None if t0_offset is None else t0_offset
        self._t: list[np.ndarray] = [] if t is None else t
        self._data: list[np.ndarray] = [] if data is None else data

    def set_t0(self, new_t0: datetime | None = None) -> None:
        """Reset the starting time of the recording.

        Parameters
        ----------
        new_t0 : datetime | None
            New starting time of the recording. If None, the starting time is set to the current time.
        """
        self._t0 = new_t0 if new_t0 is not None else datetime.now()

    def set_t0_offset(self, new_t0_offset: datetime | None = None) -> None:
        """Reset the starting time offset of the recording.

        Parameters
        ----------
        new_t0_offset : datetime | None
            New starting time offset of the recording. If None, the starting time offset is set to the current time.
        """
        if new_t0_offset is None:
            self._t0_offset = datetime.now().timestamp() - self._t0.timestamp()  # seconds
        else:
            self._t0_offset = new_t0_offset

    @property
    def t0_offset(self) -> float:
        """Get the starting time offset of the recording.

        Returns
        -------
        t : float
            Starting time offset of the recording.
        """
        return self._t0_offset

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
        if self._t0_offset is None:
            self.set_t0_offset()

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
        if self._t0_offset is None:
            self.set_t0_offset()

        self._t.append(t + self._t0_offset)
        self._data.append(data)

    def __len__(self) -> int:
        """Get the number of data blocks.

        Returns
        -------
        out : int
            Number of data blocks.
        """
        return len(self._t)

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

    def plot(self, ax=None, show: bool = True) -> None | Any:
        """Plot the data.

        Parameters
        ----------
        ax : Any
            Axes of the plot if show is False, None otherwise.
        show : bool
            Whether to show (blocking) the plot or not.

        Returns
        -------
        ax1 : Any
            Axes of the plot if show is False, None otherwise.
        """

        plt.figure("Data against time")

        # On left-hand side axes, plot nidaq data
        color = "blue"
        ax = plt.axes() if ax is None else ax
        ax.set_xlabel(f"Time [s] (first sample at {self.t0})")
        ax.set_ylabel("Data [mV]", color=color)
        ax.tick_params(axis="y", labelcolor=color)

        # We have to make sure the time and data have the same shape because new data can be added at any time
        t = self.time
        data = self.as_array.T[: t.shape[0], :]
        ax.plot(t, data, color=color)

        if show:
            plt.show(block=True)
        else:
            return ax

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

    def serialize(self, to_json: bool = False) -> dict:
        """Serialize the data.

        Parameters
        ----------
        to_json : bool
            Whether to convert numpy arrays to lists or not. If True, the data can be saved to a json file, but it
            will be slower and will not be able to be loaded back using the deserialize method.

        Returns
        -------
        out : dict
            Serialized data.
        """

        if to_json:
            t = [t.tolist() for t in self._t]
            data = [data.tolist() for data in self._data]
        else:
            t = self._t
            data = self._data

        return {"t0": self._t0.timestamp(), "t0_offset": self._t0_offset, "t": t, "data": data}

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
        out._t0 = datetime.fromtimestamp(data["t0"])
        out._t0_offset = data["t0_offset"]
        out._t = data["t"]
        out._data = data["data"]
        return out

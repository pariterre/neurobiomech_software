from copy import deepcopy
from datetime import datetime
import logging
import pickle
from typing import Any

from pyScienceMode import Channel as pyScienceModeChannel
import numpy as np


_logger = logging.getLogger("lokomat_fes")


class Channel:
    """Class to store channel information.

    Attributes
    ----------
    channel_number : int
        Channel number.
    amplitude : float
        Amplitude of the stimulation.
    """

    def __init__(self, channel_index: int, amplitude: float) -> None:
        self.channel_index = channel_index
        self.amplitude = amplitude

    @classmethod
    def from_pysciencemode(cls, channel: pyScienceModeChannel) -> "Channel":
        """Create a Channel from a pyScienceMode channel.

        Parameters
        ----------
        channel : pyScienceModeChannel
            pyScienceMode channel.

        Returns
        -------
        out : Channel
            Channel.
        """
        return cls(channel.get_no_channel(), channel.get_amplitude())

    def serialize(self, to_json: bool = False) -> dict:
        """Serialize the channel.

        Parameters
        ----------
        to_json : bool
            Whether to convert the data to json or not. In both cases, the returned dictionary will be the same. We keep
            this parameter to match the other serialize methods, in case this becomes useful in the future.

        Returns
        -------
        out : dict
            Serialized channel.
        """
        return {"channel_index": self.channel_index, "amplitude": self.amplitude}

    @classmethod
    def deserialize(cls, data):
        return cls(data["channel_index"], data["amplitude"])


class RehastimData:
    """Class to store data from Rehastim devices.

    Attributes
    ----------
    _t0 : datetime
        Starting time of the recording (set at the moment of the declaration of the class or that the
        time [set_t0] is called).
    _data: list[tuple[datetime, float, float]]
        List of data vectors.
        Each vector is a tuple of (time [datetime], duration [float] ms, tuple of channels configuration).
    """

    def __init__(self, t0: datetime = None, data: list[tuple[float, float, tuple[Channel, ...]]] = None) -> None:
        """Initialize the data.

        Parameters
        ----------
        t0 : datetime | None
            Starting time of the recording. If None, the starting time is set to the current time.
        data : list[tuple[float, float, float]] | None
            List of data vectors.
            Each vector is a tuple of (time [float], duration [float] ms, tuple of channels configuration).
        """
        self._t0: float = None
        self.set_t0(new_t0=t0)
        self._data: list[tuple[float, float, tuple[Channel, ...]]] = [] if data is None else data

    def __len__(self) -> int:
        """Get the number of samples.

        Returns
        -------
        out : int
            Number of samples.
        """
        return len(self._data)

    @property
    def has_data(self) -> bool:
        """Check if the data has been initialized.

        Returns
        -------
        out : bool
            True if the data has been initialized.
        """
        return bool(self._data)

    def clear(self) -> None:
        """Clear the data."""
        self._data = []

    def plot(self, ax=None, show: bool = True) -> None | Any:
        """Plot the data.

        Parameters
        ----------
        ax : plt.Axes | None
            Axes to plot the data to. If None, a new figure is created.
        show : bool
            Whether to show the plot or not.
        """
        import matplotlib.pyplot as plt

        ax = plt.axes() if ax is None else ax

        color = "red"
        ax.set_ylabel("Amplitude [mA]", color=color)
        ax.tick_params(axis="y", labelcolor=color)
        all_time = self.time - self._t0
        if not all_time.any():
            return
        all_duration = self.duration_as_array[: all_time.shape[0]]
        all_amplitude = self.amplitude_as_array.T[: all_time.shape[0], :]
        for time, duration, amplitude in zip(all_time, all_duration, all_amplitude):
            plt.plot(
                [time, time + duration],
                np.concatenate((amplitude[np.newaxis, :], amplitude[np.newaxis, :])),
                color=color,
            )

        if show:
            plt.show()
        else:
            return ax

    @property
    def t0(self) -> datetime:
        """Get the starting time of the recording.

        Returns
        -------
        out : datetime
            Starting time of the recording.
        """
        return datetime.fromtimestamp(self._t0)

    def set_t0(self, new_t0: datetime | None = None) -> None:
        """Reset the starting time of the recording.

        Parameters
        ----------
        new_t0 : datetime | None
            New starting time of the recording. If None, the starting time is set to the current time.
        """
        self._t0 = (new_t0 if new_t0 is not None else datetime.now()).timestamp()

    def add(
        self, now: float, duration: float | None, channels: tuple[Channel | pyScienceModeChannel, ...] | None
    ) -> None:
        """Add data from a Rehastim device to the data.

        Parameters
        ----------
        now : float
            Timestamp of the data.
        duration : float
            Duration of the stimulation. If duration is set to None, the value is changed on the next start.
        amplitude : float
            Amplitude of the stimulation.
        """
        if channels is None:
            # Copy the previous values
            if not self.has_data:
                raise RuntimeError("The first time you add data, you must specify the channels.")
            channels = self._data[-1][2]
        else:
            # Make a copy of the channels to avoid modifying the original, if the channel is a pyScienceModeChannel
            # we convert it to the simplified version (Channel)
            channels = tuple(
                Channel.from_pysciencemode(channel) if isinstance(channel, pyScienceModeChannel) else deepcopy(channel)
                for channel in channels
            )
        if len(self._data) > 0 and self._data[-1][1] is None:
            # If the previous duration was None, we set it to the current time
            self._data[-1] = (self._data[-1][0], now - self._data[-1][0], self._data[-1][2])

        self._data.append((now, duration, channels))

    def sample_block(
        self, index: int | slice
    ) -> tuple[datetime, float, tuple[Channel, ...]] | list[tuple[datetime, float, tuple[Channel, ...]]]:
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
            return None

        return self._data[index]

    def sample_block_between(self, t0: float, tf: float) -> list[tuple[datetime, float, tuple[Channel, ...]]]:
        """Get a block of data between two times.

        Parameters
        ----------
        t0 : float
            Starting time.
        tf : float
            Ending time.

        Returns
        -------
        out : list[tuple[datetime, float, float]]
            List of data vectors.
            Each vector is a tuple of (time [datetime], duration [float] ms, tuple of channels configuration).
        """
        if not self._data:
            return []

        time = self.time

        first_index = None
        for i in range(len(time)):
            if time[i] >= t0:
                first_index = i
                break
        else:
            return []

        last_index = 0
        for i in reversed(range(len(time))):
            if time[i] <= tf:
                last_index = i
                break

        return self.sample_block(slice(first_index, last_index + 1))

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

        return np.array([t for t, duration, _ in self._data if duration is not None])

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

        return np.array([duration for _, duration, _ in self._data if duration is not None])

    @property
    def amplitude_as_array(self) -> np.ndarray:
        """Get amplitude data to a numpy array (n_channels x n_samples).

        Parameters
        ----------

        Returns
        -------
        data : np.ndarray
            Amplitude of each stimulation
        """
        if not self._data:
            return np.array([[]])

        return np.array(
            [
                [channel.amplitude for channel in channels]
                for _, duration, channels in self._data
                if duration is not None
            ]
        ).T

    @property
    def copy(self) -> "RehastimData":
        """Get a copy of the data.

        Returns
        -------
        out : RehastimData
            Copy of the data.
        """

        out = RehastimData()
        out._t0 = self._t0
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
            pickle.dump(self.serialize(), f)

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

    def serialize(self, to_json: bool = False) -> dict:
        """Serialize the data.

        Parameters
        ----------
        to_json : bool
            Whether to convert the data to json or not. If False, the numpy arrays are kept as numpy arrays. If True,
            they are converted to lists. Default is False. Note that this is only useful if you want to save the data
            to a json file. The resulting dictionary will not be able to be deserialized using the deserialize method.

        Returns
        -------
        out : dict
            Serialized data.
        """
        if to_json:
            data = [list((d[0], d[1], tuple(channel.serialize(to_json=True) for channel in d[2]))) for d in self._data]
        else:
            data = self._data
        return {"t0": self._t0, "data": data}

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
        out._t0 = data["t0"]
        out._data = data["data"]
        return out

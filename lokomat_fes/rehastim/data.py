from copy import deepcopy
from datetime import datetime
import pickle

from pyScienceMode import Channel as pyScienceModeChannel
import numpy as np


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

    @property
    def serialized(self):
        return {"channel_index": self.channel_index, "amplitude": self.amplitude}

    @classmethod
    def deserialize(cls, data):
        return cls(data["channel_index"], data["amplitude"])


class RehastimData:
    """Class to store data from Rehastim devices.

    Attributes
    ----------
    _data: list[tuple[datetime, float, float]]
        List of data vectors.
        Each vector is a tuple of (time [datetime], duration [float] ms, tuple of channels configuration).
    """

    def __init__(self) -> None:
        self._data: list[tuple[datetime, float, tuple[Channel, ...]]] = []

    @property
    def has_data(self) -> bool:
        """Check if the data has been initialized.

        Returns
        -------
        out : bool
            True if the data has been initialized.
        """
        return bool(self._data)

    def add(self, duration: float, channels: tuple[Channel | pyScienceModeChannel, ...] | None) -> None:
        """Add data from a Rehastim device to the data.

        Parameters
        ----------
        duration : float
            Duration of the stimulation.
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
        self._data.append((datetime.now(), duration, channels))

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

        return np.array([[channel.amplitude for channel in channels] for _, _, channels in self._data]).T

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
            pickle.dump(self.serialized, f)

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
    def serialized(self) -> dict:
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

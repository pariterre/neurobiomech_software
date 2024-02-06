from datetime import datetime
import pickle

import matplotlib.pyplot as plt

from ..nidaq.data import NiDaqData
from ..rehastim.data import RehastimData


class Data:
    def __init__(self, nidaq: NiDaqData = None, rehastim: RehastimData = None, t0: datetime = None) -> None:
        """Initialize the data."""
        self.nidaq = NiDaqData() if nidaq is None else nidaq
        self.rehastim = RehastimData() if rehastim is None else rehastim
        self._t0: float = None
        self.set_t0(new_t0=t0)  # Make sure all the time corresponds

    def __len__(self) -> int:
        """Get the length of the data.

        Returns
        -------
        out : int
            Length of the data.
        """

        return len(self.nidaq)

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
        """Reset the time.

        Parameters
        ----------
        new_t0 : datetime | None
            New starting time of the recording. If None, the starting time is set to the current time.
        """
        if new_t0 is None:
            new_t0 = datetime.now()

        self._t0 = new_t0.timestamp()
        self.nidaq.set_t0(new_t0=new_t0)
        self.rehastim.set_t0(new_t0=new_t0)

    def clear(self) -> None:
        """Clear the data."""
        self.nidaq.clear()
        self.rehastim.clear()
        self.set_t0()

    @property
    def copy(self) -> "Data":
        """Copy the data.

        Returns
        -------
        out : Data
            Copy of the data.
        """

        out = Data()
        out.nidaq = self.nidaq.copy
        out.rehastim = self.rehastim.copy
        return out

    def serialize(self, to_json: bool = False) -> dict:
        """Serialize the data to a dictionary.

        Parameters
        ----------
        to_json : bool
            Whether to convert the data to json or not. If False, the numpy arrays are kept as numpy arrays. If True,
            they are converted to lists. Default is False. Note that this is only useful if you want to save the data
            to a json file. The resulting dictionary will not be able to be deserialized using the deserialize method.

        Returns
        -------
        out : dict
            Dictionary with the data.
        """

        return {
            "t0": self._t0,
            "nidaq": self.nidaq.serialize(to_json=to_json),
            "rehastim": self.rehastim.serialize(to_json=to_json),
        }

    @classmethod
    def deserialize(cls, data: dict) -> "Data":
        """Deserialize the data from a dictionary.

        Parameters
        ----------
        data : dict
            Dictionary with the data.

        Returns
        -------
        out : Data
            Data object.
        """

        out = cls()
        out.nidaq = NiDaqData.deserialize(data["nidaq"])
        out.rehastim = RehastimData.deserialize(data["rehastim"])
        out.set_t0(new_t0=datetime.fromtimestamp(datetime.fromtimestamp(data["t0"])))
        return out

    def plot(self, show: bool = True) -> None:
        """Plot the data.

        Parameters
        ----------
        show : bool
            Whether to show (blocking) the plot or not.
        """

        plt.figure("Data against time")

        # On left-hand side axes, plot nidaq data
        ax1 = self.nidaq.plot(show=False)

        # On right-hand side axes, plot rehastim data as stair data (from t0 to duration at height of amplitude)
        self.rehastim.plot(ax=ax1.twinx(), show=False)

        if show:
            plt.show()

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
    def load(cls, path: str) -> "Data":
        """Load the data from a file.

        Parameters
        ----------
        path : str
            Path to the file.

        Returns
        -------
        out : Data
            Loaded data.
        """

        with open(path, "rb") as f:
            data = pickle.load(f)
        return cls.deserialize(data)

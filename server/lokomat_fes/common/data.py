from datetime import datetime
import pickle

from ..nidaq.data import NiDaqData
from ..rehastim.data import RehastimData


class Data:
    def __init__(self) -> None:
        """Initialize the data."""
        self.nidaq = NiDaqData()
        self.rehastim = RehastimData()
        self._t0: datetime = datetime.now()
        self.set_t0(new_t0=self._t0)  # Just make sure t0 is the exact same for both devices

    @property
    def t0(self) -> datetime:
        """Get the starting time of the recording.

        Returns
        -------
        out : datetime
            Starting time of the recording.
        """

        return self._t0

    def set_t0(self, new_t0: datetime | None = None) -> None:
        """Reset the time.

        Parameters
        ----------
        new_t0 : datetime | None
            New starting time of the recording. If None, the starting time is set to the current time.
        """
        if new_t0 is None:
            new_t0 = datetime.now()

        self._t0 = new_t0
        self.nidaq.set_t0(new_t0=self._t0)
        self.rehastim.set_t0(new_t0=self._t0)

    def add_nidaq_data(self, t: float, data: float) -> None:
        """Add data to the NiDaq data.

        Parameters
        ----------
        t : float
            Time.
        data : float
            Data.
        """

        self.nidaq.add(t, data)

    def add_rehastim_data(self, duration: float, amplitude: float) -> None:
        """Add data to the Rehastim data.

        Parameters
        ----------
        duration : float
            Duration of the stimulation.
        amplitude : float
            Amplitude of the stimulation.
        """
        if not self.nidaq.has_data:
            raise ValueError("Synchronising rehastim data with nidaq requires that nidaq has data first")

        self.rehastim.add(duration, amplitude)

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

    @property
    def serialized(self) -> dict:
        """Serialize the data to a dictionary.

        Returns
        -------
        out : dict
            Dictionary with the data.
        """

        return {
            "nidaq": self.nidaq.serialized,
            "rehastim": self.rehastim.serialized,
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

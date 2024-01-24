from abc import ABC, abstractmethod
import logging

from ..common.data import Data
from ..nidaq import NiDaqGeneric
from ..rehastim import RehastimGeneric

logger = logging.getLogger("lokomat_fes")


class GuiGeneric(ABC):
    """Abstract base class for GUIs."""

    def __init__(self, rehastim: RehastimGeneric, nidaq: NiDaqGeneric) -> None:
        """Initialize the GUI."""
        logger.info("Initializing the GUI")

        self._rehastim = rehastim
        self._nidaq = nidaq
        self._data = None

    def exec(self):
        """Start the GUI."""
        logger.info("Starting the GUI")
        self._exec()

    def _prepare_data(self):
        """Prepare the data."""
        logger.info("Preparing the data")
        self._data = Data()

        # Initialize the callback to record the data
        self._nidaq.register_to_data_ready(self._data.nidaq.add_sample_block)
        self._rehastim.register_to_on_stimulation_started(self._data.rehastim.add)

    def _finalize_data(self):
        """Finalize the data."""
        logger.info("Finalizing the data")
        self._nidaq.unregister_to_data_ready(self._data.nidaq.add_sample_block)
        self._rehastim.unregister_to_on_stimulation_started(self._data.rehastim.add)

    def _start_recording(self):
        """Start the recording."""
        logger.info("Starting recording")
        self._prepare_data()
        self._nidaq.start_recording()

    def _stop_recording(self):
        """Stop the recording."""
        logger.info("Stopping recording")
        self._finalize_data()

        self._rehastim.stop_stimulation()  # Interrupt any active stimulation if needed
        self._nidaq.stop_recording()

    def _start_stimulation(self):
        """Start the Rehastim stimulation."""
        logger.info("Starting Rehastim stimulation")
        self._rehastim.start_stimulation()

    def _stop_stimulation(self):
        """Stop the Rehastim stimulation."""
        logger.info("Stopping Rehastim stimulation")
        self._rehastim.stop_stimulation()

    @abstractmethod
    def _exec(self) -> None:
        """Start the GUI (implementation)."""

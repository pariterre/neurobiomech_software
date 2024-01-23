from abc import ABC, abstractmethod
import logging

from ..rehastim import RehastimGeneric
from ..nidaq import NiDaqGeneric

logger = logging.getLogger("lokomat_fes")


class GuiGeneric(ABC):
    """Abstract base class for GUIs."""

    def __init__(self, rehastim: RehastimGeneric, nidaq: NiDaqGeneric) -> None:
        """Initialize the GUI."""
        logger.info("Initializing the GUI")

        self._rehastim = rehastim
        self._nidaq = nidaq

    def exec(self):
        """Start the GUI."""
        logger.info("Starting the GUI")
        self._exec()

    def _start_nidaq_recording(self):
        """Start the NI-DAQ recording."""
        logger.info("Starting NI-DAQ recording")
        self._nidaq.start_recording()

    def _stop_nidaq_recording(self):
        """Stop the NI-DAQ recording."""
        logger.info("Stopping NI-DAQ recording")
        self._nidaq.stop_recording()

    def _start_rehastim_stimulation(self):
        """Start the Rehastim stimulation."""
        logger.info("Starting Rehastim stimulation")
        self._rehastim.start_stimulation()

    def _stop_rehastim_stimulation(self):
        """Stop the Rehastim stimulation."""
        logger.info("Stopping Rehastim stimulation")
        self._rehastim.stop_stimulation()

    def _stop_devices(self):
        """Stop the devices."""
        self._stop_nidaq_recording()
        self._stop_rehastim_stimulation()

    @abstractmethod
    def _exec(self) -> None:
        """Start the GUI (implementation)."""

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

    @abstractmethod
    def exec(self):
        """Start the GUI."""

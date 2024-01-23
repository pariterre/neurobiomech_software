from typing import override

from .gui_generic import GuiGeneric
from ..common.logger import logger


class GuiConsole(GuiGeneric):
    """Console GUI."""

    @override
    def exec(self):
        """Start the GUI."""
        logger.info("Starting console GUI.")
        print("Press s to start stimulation, q to stop stimulation: ")
        while True:
            key = input()
            if key == "s":
                print("Stimulating!")

            elif key == "q":
                break

        logger.info("Console GUI exited.")

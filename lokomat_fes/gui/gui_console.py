import logging
from typing import override

from .gui_generic import GuiGeneric

logger = logging.getLogger("lokomat_fes")


class GuiConsole(GuiGeneric):
    """Console GUI."""

    @override
    def exec(self):
        """Start the GUI."""
        logger.info("Starting console GUI.")

        print("Send 's' to start stimulation, 'q' to stop stimulation: ")
        while True:
            key = input()
            if key == "s":
                duration = input("How long should the stimulation last? (in seconds): ")
                try:
                    duration = float(duration)
                except:
                    logger.exception("Invalid duration.")
                    continue

                self._rehastim.start_stimulation(duration=duration)

            elif key == "q":
                break

        logger.info("Console GUI exited.")

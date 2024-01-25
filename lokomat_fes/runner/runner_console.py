import logging
from typing import override

from .runner_generic import RunnerGeneric

logger = logging.getLogger("lokomat_fes")


class RunnerConsole(RunnerGeneric):
    """Run the program in a local console."""

    @override
    def _exec(self):
        """Start the runner."""
        logger.info("Starting console runner.")

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

        logger.info("Runner Console exited.")

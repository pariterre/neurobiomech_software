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

        print("Type your command and press enter, use 'list' the print the commands:")
        while True:
            request = input().lower()
            command = request.split(" ")[0]
            parameters = request.split(" ")[1:]

            if command == "list":
                print("List of commands:")
                print("\tlist: list all the commands")
                print("\tstart: start recording")
                print("\tstop: stop recording")
                print("\tstim X: stimulate for X seconds")
                print("\tquit: quit")

            elif command == "start":
                self._start_recording()
                print("Recording started.")

            elif command == "stop":
                self._stop_recording()
                print("Recording stopped.")

            elif command == "stim":
                duration = parameters[0]
                try:
                    duration = float(duration)
                except:
                    logger.exception("Invalid duration, it must be a float.")
                    continue

                self._rehastim.start_stimulation(duration=duration)

            elif command == "quit":
                break

        logger.info("Runner Console exited.")

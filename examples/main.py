import logging

import numpy as np
from lokomat_fes import setup_logger, RunnerConsole, StrideBasedStimulation, Side
from lokomat_fes.lokomat import NiDaqLokomat, RehastimLokomat

# If you want to use the real devices, comment the following lines
from lokomat_fes.lokomat import NiDaqLokomatMock as NiDaqLokomat
from lokomat_fes.lokomat import RehastimLokomatMock as RehastimLokomat

logger = logging.getLogger("runner")


def _received_data(t: np.ndarray, data: np.ndarray) -> None:
    """Callback function that is called when new nidaq data are received"""

    t_index = 0
    hip_index = 0
    logger.info(f"Received data, at {t[t_index]}, initial hip angle: {data[hip_index, t_index]}")


def __main__() -> None:
    """Main function"""
    setup_logger(level=logging.WARNING, show_exec=False)  # Change to logging.INFO to see more logs

    # Define the devices and the runner
    rehastim = RehastimLokomat()
    nidaq = NiDaqLokomat(time_between_samples=0.1, frame_rate=1000)
    runner = RunnerConsole(rehastim, nidaq)

    # Plan some stimulations based on where we are in the stride cycle
    runner.schedule_stimulation(StrideBasedStimulation.stimulate_in_swing_phase(side=Side.LEFT))

    # This is to monitor the data. Note you can do that even when using exec() to monitor data while using the Runner
    nidaq.register_to_data_ready(_received_data)

    # Start the runner (blocking)
    runner.exec()
    # # Alternatively, you can use a blocking sleep to manually manage the devices
    # nidaq.start_recording()  # Start the devices (only necessary if you use the manual mode)
    # sleep(2)
    # rehastim.start_stimulation(duration=None)  # Start a stimulation
    # sleep(1)
    # rehastim.stop_stimulation()  # Stop the stimulation
    # sleep(2)
    # nidaq.stop_recording()  # Stop the devices (only necessary if you use the manual mode)

    # # You can manipulate the last trial like so (includes Nidaq and Rehastim data)
    # data = runner.last_trial
    # if data is not None:
    #     # You can save and reload them
    #     data.save("data.pkl")
    #     data_loaded = Data.load("data.pkl")

    #     # And you can plot them (or do whatever you want with them)
    #     # Note that [data_loaded] is a valid instance of the data class (you can either use the one you saved or the
    #     # one you got from the runner, they are virtually the same)
    #     runner.plot_data(data_loaded)

    #     # That said, you can plot the data using the plot method of the ConsoleRunner.

    # Dispose the devices (this is important to avoid memory leaks and stop subprocesses)
    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

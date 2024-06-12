import logging
from time import sleep

import numpy as np
from lokomat_fes import setup_logger, RunnerTcp, Side, Data
from lokomat_fes.lokomat import NiDaqLokomat, RehastimLokomat

# If you want to use the real devices, comment the following lines
from lokomat_fes.lokomat import NiDaqLokomatMock as NiDaqLokomat
from lokomat_fes.lokomat import RehastimLokomatMock as RehastimLokomat

_logger = logging.getLogger("lokomat_fes")


def _received_data(t: np.ndarray, data: np.ndarray) -> None:
    """Callback function that is called when new nidaq data are received"""

    t_index = 0
    hip_index = 0
    _logger.info(f"Received data, at {t[t_index]}, initial hip angle: {data[hip_index, t_index]}")


def __main__() -> None:
    """Main function"""
    # Interesting levels for logging are: INFO, WARN
    setup_logger(lokomat_fes_logging_level=logging.INFO, pyscience_logging_level=logging.WARN)

    # Define the devices and the runner
    rehastim = RehastimLokomat(port="NoPort")
    nidaq = NiDaqLokomat(time_between_samples=0.1, frame_rate=1000)
    # runner = RunnerConsole(rehastim, nidaq)
    runner = RunnerTcp(rehastim=rehastim, nidaq=nidaq)

    # Load the stimulation rules
    # runner.schedule_stimulation(runner.available_schedules[0])

    # Plan some stimulations based on where we are in the stride cycle
    # runner.schedule_stimulation(StrideBasedStimulation.stimulate_in_swing_phase(side=Side.LEFT))

    # This is to monitor the data. Note you can do that even when using exec() to monitor data while using the Runner
    # nidaq.register_to_data_ready(_received_data)

    # Start the runner (blocking)
    runner.exec()
    # Alternatively, you can use a blocking sleep to manually manage the devices
    # runner.start_nidaq()  # Start the devices (only necessary if you use the manual mode)
    # runner.start_recording()
    # sleep(2)
    # runner.start_stimulation(duration=None)  # Start a stimulation
    # sleep(1)
    # runner.stop_stimulation()  # Stop the stimulation
    # sleep(2)
    # runner.stop_recording()  # Stop the devices (only necessary if you use the manual mode)
    # runner.stop_nidaq()

    # You can manipulate the last trial like so (includes Nidaq and Rehastim data)
    # data = runner.last_trial
    # if data is not None:
    #     # You can save and reload them
    #     data.save("data.pkl")
    #     data_loaded = Data.load("data.pkl")

    #     # And you can plot them (or do whatever you want with them)
    #     data_loaded.plot()
    #     # Note that [data_loaded] is a valid instance of the data class (you can either use the one you saved or the
    #     # one you got from the runner, they are virtually the same)
    #     runner.plot_data()

    #     # That said, you can plot the data using the plot method of the ConsoleRunner.

    # Dispose the devices (this is important to avoid memory leaks and stop subprocesses)
    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

from time import sleep
import logging

from matplotlib import pyplot as plt
from lokomat_fes import setup_logger, Data
from lokomat_fes.runner import RunnerConsole
from lokomat_fes.nidaq.data import NiDaqData
from lokomat_fes.nidaq import NiDaqLokomat
from lokomat_fes.rehastim import RehastimLokomat

# If you want to use the real devices, comment the following lines
from lokomat_fes.nidaq.mocks import NiDaqLokomatMock as NiDaqLokomat
from lokomat_fes.rehastim.mocks import RehastimLokomatMock as RehastimLokomat


logger = logging.getLogger("lokomat_fes")


def _received_data(t, data) -> None:
    """Callback function that is called when new data are received"""

    t_index = 0
    hip_index = 0
    print(f"Received data, at {t[t_index]}, initial hip angle: {data[hip_index, t_index]}")


def plot_data(data: NiDaqData) -> None:
    plt.figure("Data against time")
    ax = plt.axes()
    ax.set_xlabel(f"Time [s] (first sample at {data.start_recording_time})")
    ax.set_ylabel("Data (mV)")
    plt.plot(data.time, data.as_array.T)
    plt.show()


def __main__() -> None:
    setup_logger(level=logging.WARNING)  # Change to logging.INFO to see more logs

    # Define the devices and the runner
    rehastim = RehastimLokomat()
    nidaq = NiDaqLokomat()
    nidaq.register_to_data_ready(_received_data)  # This is to monitor the data. It is not necessary if you use a runner
    runner = RunnerConsole(rehastim, nidaq)

    # Start the devices (this is not necessary if exec() is called)
    # nidaq.start_recording()

    # Start the runner (blocking)
    runner.exec()
    # Alternatively, you can use a blocking sleep to wait for the devices to record
    # sleep(1)

    # Stop the devices (this is not necessary if exec() or dispose() is called)
    # nidaq.stop_recording()
    # rehastim.stop_stimulation()

    # Get the last recorded data
    data = nidaq.data

    # You can also save and reload
    data.save("data.pkl")
    data_loaded = NiDaqData.load("data.pkl")

    # You can plot pretty easily, note that data_loaded is a valid class instance like the data
    plot_data(data_loaded)

    # Dispose the devices (this is important to avoid memory leaks and stop subprocesses)
    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

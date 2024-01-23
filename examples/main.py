# from time import sleep

from matplotlib import pyplot as plt

from lokomat_fes.gui import GuiConsole

# Load the NiDaq device
from lokomat_fes.nidaq.data import NiDaqData

# from lokomat_fes.nidaq import NiDaqLokomat
from lokomat_fes.nidaq.mocks import NiDaqLokomatMock as NiDaqLokomat

# Load the Rehastim device
# from lokomat_fes.rehastim import RehastimLokomat
from lokomat_fes.rehastim.mocks import RehastimLokomatMock as RehastimLokomat


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
    # Define the devices
    rehastim = RehastimLokomat()
    nidaq = NiDaqLokomat(on_data_ready=_received_data)
    gui = GuiConsole(rehastim, nidaq)

    # Start the devices
    nidaq.start_recording()
    rehastim.start_stimulation(duration=1)

    # Start the GUI (blocking)
    gui.exec()
    # Alternatively, you can use a blocking sleep to wait for the devices to record
    # sleep(1)

    # Stop the devices (this is not necessary if dispose() is called)
    nidaq.stop_recording()
    rehastim.stop_stimulation()

    # Get and manipulate the data
    data = nidaq.data
    plot_data(data)

    # Save and load
    # data.save("data.pkl")
    # data_loaded = NiDaqData.load("data.pkl")

    # Dispose the devices (this is important to avoid memory leaks and stop subprocesses)
    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

from time import sleep

from lokomat_fes.gui import GuiConsole

# Load the NiDaq device
# from lokomat_fes.nidaq import NiDaqLokomat
from lokomat_fes.nidaq.mocks import NiDaqLokomatMock as NiDaqLokomat

# Load the Rehastim device
# from lokomat_fes.rehastim import RehastimLokomat
from lokomat_fes.rehastim.mocks import RehastimLokomatMock as RehastimLokomat


def _received_data(t, data):
    pass
    # print(f"Received data, first is {t[-1][0], data[0][:, 0]}")


def __main__():
    # Define the devices
    rehastim = RehastimLokomat()
    nidaq = NiDaqLokomat(on_data_ready_callback=_received_data)
    gui = GuiConsole(rehastim, nidaq)

    # Start the devices
    nidaq.start_recording()
    rehastim.start_stimulation(duration=1)

    # Start the GUI (blocking)
    gui.exec()
    # Alternatively, you can use a blocking sleep to wait for the devices to record
    # sleep(1)

    # Stop the devices (this is not necessary if dispose() is called)
    # nidaq.stop_recording()
    # rehastim.stop_stimulation()

    # Stop and dispose the devices (this is important to avoid memory leaks and stop subprocesses)
    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

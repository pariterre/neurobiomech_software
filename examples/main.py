from time import sleep

from lokomat_fes.rehastim import LokomatRehastim
from lokomat_fes.nidaq import LokomatNiDaqMock


def _received_data(t, data):
    print(f"Received data, first is {t[-1][0], data[0][:, 0]}")


def __main__():
    rehastim = LokomatRehastim()
    rehastim.start_device()

    nidaq = LokomatNiDaqMock(on_data_ready_callback=_received_data)

    nidaq.start_recording()
    rehastim.start_stimulation(duration=1)

    sleep(5)

    nidaq.stop_recording()
    rehastim.end_stimulation()

    rehastim.dispose()
    nidaq.dispose()

    # The current implementation does not exit properly


if __name__ == "__main__":
    __main__()

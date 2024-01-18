from time import sleep

from lokomat_fes.rehastim import LokomatRehastimMock
from lokomat_fes.nidaq import LokomatNiDaqMock


def _received_data(t, data):
    print(f"Received data, first is {t[-1][0], data[0][:, 0]}")


def __main__():
    rehastim = LokomatRehastimMock()
    nidaq = LokomatNiDaqMock(on_data_ready_callback=_received_data)

    nidaq.start_recording()

    sleep(5)

    nidaq.stop_recording()

    rehastim.dispose()
    nidaq.dispose()


if __name__ == "__main__":
    __main__()

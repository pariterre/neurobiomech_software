from time import sleep

from lokomat_fes.rehastim.mocks import LokomatRehastimMock as LokomatRehastim

from lokomat_fes.nidaq import LokomatNiDaq

# from lokomat_fes.nidaq.mocks import LokomatNiDaqMock as LokomatNiDaq


def _received_data(t, data):
    print(f"Received data, first is {t[-1][0], data[0][:, 0]}")


def __main__():
    rehastim = LokomatRehastim()
    nidaq = LokomatNiDaq(on_data_ready_callback=_received_data)

    nidaq.start_recording()
    rehastim.start_stimulation(duration=1)

    sleep(1)

    nidaq.stop_recording()
    rehastim.stop_stimulation()

    sleep(1)
    nidaq.start_recording()
    rehastim.start_stimulation(duration=1)

    sleep(1)

    nidaq.dispose()
    rehastim.dispose()


if __name__ == "__main__":
    __main__()

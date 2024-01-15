from lokomat_fes import LokomatNiDaqMock


def test_nidaq_initialize():
    nidaq = LokomatNiDaqMock()
    assert nidaq.num_channels == 16
    assert nidaq.frames_per_second == 1000
    assert nidaq.dt == 1 / 1000

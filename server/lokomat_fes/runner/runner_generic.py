from abc import ABC, abstractmethod
import logging

from ..common.data import Data
from ..nidaq import NiDaqGeneric, NiDaqData
from ..rehastim import RehastimGeneric, RehastimData
from ..scheduler.scheduler import Scheduler
from ..scheduler.stimulation import StimulationAbstract

_logger = logging.getLogger("lokomat_fes")


class RunnerGeneric(ABC):
    """Abstract base class for Runners."""

    def __init__(self, rehastim: RehastimGeneric, nidaq: NiDaqGeneric) -> None:
        """Initialize the Runner."""
        _logger.info("Initializing the Runner")

        self._rehastim = rehastim
        self._nidaq = nidaq

        self._continuous_data = Data()
        self._register_data_to_callbacks(self._continuous_data)
        self._last_fetch_continuous_data_index = -1

        self._scheduler = Scheduler(runner=self, data=self._continuous_data)

        self._trial_data = None
        self._is_recording = False

    def exec(self):
        """Start the Runner."""
        _logger.info("Starting the Runner")
        self._exec()

        # Make sure the devices are stopped
        if self._is_recording:
            self.stop_recording()

        self._scheduler.dispose()

    @abstractmethod
    def _exec(self) -> None:
        """Start the Runner (implementation)."""

    ### DATA RELATED METHODS ###
    def plot_data(self) -> None:
        """Plot the data."""

        _logger.info("Plotting the data")
        data = self._trial_data
        if data is None:
            _logger.error("No data to plot.")
            return False

        data.plot()

        return True

    def save_trial(self, filename: str) -> None:
        """Save the last recorded trial.

        Parameters
        ----------
        filename : str
            The filename to save the trial to.
        """
        _logger.info(f"Saving the last recorded trial to {filename}")

        if self._trial_data is None:
            _logger.error("Cannot save, no trial recorded yet")
            raise RuntimeError("Cannot save, no trial recorded yet")
        if self._is_recording:
            _logger.error("Cannot save while recording")
            raise RuntimeError("Cannot save while recording")

        self._trial_data.save(filename)

    @property
    def last_trial(self) -> Data | None:
        """Get the last recorded trial.

        Returns
        -------
        Data
            The last recorded trial.
        """
        return self._trial_data

    def _start_fetch_continuous_data(self) -> None:
        """Start fetching the continuous data."""
        _logger.info("Starting to fetch the continuous data")

        if self._continuous_data is None:
            _logger.error("Cannot fetch continuous data while no data is recorded")
            raise RuntimeError("Cannot fetch continuous data while no data is recorded")
        self._last_fetch_continuous_data_index = len(self._continuous_data)

    def _fetch_continuous_data(self, from_top: bool = False) -> Data:
        """Fetch the last recorded trial.

        Parameters
        ----------
        from_top : bool
            Whether to fetch the data from where we last fetched it or from the top.

        Returns
        -------
        Data
            The last recorded trial.
        """
        _logger.info("Fetching the continuous data")

        if not self._nidaq.is_connected:
            _logger.error("Cannot fetch continuous data while the NiDaq is not connected")
            raise RuntimeError("Cannot fetch continuous data while the NiDaq is not connected")

        # Fetch the NiDaq data
        last_data_index = len(self._continuous_data.nidaq) - 1
        if self._last_fetch_continuous_data_index == last_data_index:
            # No new data
            return Data()
        starting_index = 0 if from_top else (self._last_fetch_continuous_data_index + 1)
        nidaq = self._continuous_data.nidaq.sample_block(slice(starting_index, last_data_index + 1))

        # Fetch the corresponding Rehastim data (comprised between the first and last NiDaq data)
        rehastim = self._continuous_data.rehastim.sample_block_between(t0=nidaq[0][0][0], tf=nidaq[0][-1][-1])

        # Update the last fetched data index
        self._last_fetch_continuous_data_index = last_data_index

        # Return a new Data object with the fetched data (t0 is still the real t0 though)
        t0 = self._continuous_data.t0
        nidaq_data = NiDaqData(t0=t0, t0_offset=self._continuous_data.nidaq.t0_offset, t=nidaq[0], data=nidaq[1])
        rehastim_data = RehastimData(t0=t0, data=rehastim)
        return Data(nidaq=nidaq_data, rehastim=rehastim_data, t0=self._continuous_data.t0)

    def _prepare_trial(self):
        """Prepare the data."""
        _logger.info("Preparing the data")
        self._trial_data = Data()

        # Initialize the callback to record the data
        self._register_data_to_callbacks(self._trial_data)

    def _register_data_to_callbacks(self, data: Data):
        """Register the data to the callbacks."""
        self._nidaq.register_to_data_ready(data.nidaq.add_sample_block)
        self._rehastim.register_to_on_stimulation_started(data.rehastim.add)
        self._rehastim.register_to_on_stimulation_stopped(data.rehastim.stop_undefined_stimulation_duration)

    def _unregister_data_to_callbacks(self, data: Data):
        """Unregister the data to the callbacks."""
        self._nidaq.unregister_to_data_ready(data.nidaq.add_sample_block)
        self._rehastim.unregister_to_on_stimulation_started(data.rehastim.add)
        self._rehastim.unregister_to_on_stimulation_stopped(data.rehastim.stop_undefined_stimulation_duration)

    def _finalize_trial(self):
        """Finalize the data."""
        _logger.info("Finalizing the data")

        self._unregister_data_to_callbacks(self._trial_data)

    ### KINEMATIC DEVICE (NIDAQ) RELATED METHODS ###
    def start_nidaq(self):
        """Start the NiDaq."""
        _logger.info("Starting NiDaq")
        self._nidaq.connect()

    def stop_nidaq(self):
        """Stop the NiDaq."""
        _logger.info("Stopping NiDaq")
        self._nidaq.disconnect()

    def start_recording(self):
        _logger.info("Starting recording.")
        if not self._nidaq.is_connected:
            self.start_nidaq()

        if self._is_recording:
            _logger.error("Cannot start recording while already recording")
            raise RuntimeError("Cannot start recording while already recording")

        """Start the recording."""
        self._prepare_trial()
        self._nidaq.start_recording()

        self._is_recording = True

    def stop_recording(self):
        """Stop the recording."""
        _logger.info("Stopping recording.")
        if not self._is_recording:
            _logger.error("Cannot stop recording while not recording")
            raise RuntimeError("Cannot stop recording while not recording")

        self._finalize_trial()

        self._rehastim.stop_stimulation()  # Interrupt any active stimulation if needed
        self._nidaq.stop_recording()
        self._is_recording = False

    ### STIMULATION DEVICE (REHASTIM) RELATED METHODS ###
    def schedule_stimulation(self, stimulation: StimulationAbstract):
        """Schedule a stimulation.

        Parameters
        ----------
        duration : float
            The duration of the stimulation.
        amplitude : float
            The amplitude of the stimulation.
        """
        _logger.info(f"Scheduling (index={len(self._scheduler)}) the stimulation {stimulation} ")
        self._scheduler.add(stimulation=stimulation)

    def get_scheduled_stimulations(self) -> list[StimulationAbstract]:
        """Get the scheduled stimulations.

        Returns
        -------
        list[StimulationAbstract]
            The scheduled stimulations.
        """
        _logger.info("Getting all the scheduled stimulations")
        return self._scheduler.get_stimulations()

    def remove_scheduled_stimulation(self, index: int):
        """Remove a scheduled stimulation. To list the currently scheduled stimulations, use get_scheduled_stimulations().

        Parameters
        ----------
        index : int
            The index of the stimulation to remove.
        """
        _logger.info(f"Removing the scheduled stimulation at index={index}")
        self._scheduler.remove(stimulation=self._scheduler.get_stimulations()[index])

    def start_stimulation(self, duration: float):
        """Start the Rehastim stimulation."""
        amplitude = self._rehastim.get_pulse_amplitude()[0]
        width = self._rehastim.get_pulse_width()[0]
        _logger.info(f"Starting Rehastim stimulation for {duration}s at {amplitude}mA and {width}ms.")

        self._rehastim.start_stimulation(duration)

    def stop_stimulation(self):
        """Stop the Rehastim stimulation."""
        _logger.info("Stopping Rehastim stimulation")
        self._rehastim.stop_stimulation()

    def set_stimulation_pulse_amplitude(self, amplitudes: float | list[float]):
        """Set the Rehastim stimulation amplitude.

        Parameters
        ----------
        amplitudes : float | list[float]
            The amplitude to set. If a list is provided, the amplitude is set for each channel.
        """
        _logger.info("Setting Rehastim stimulation amplitude")
        self._rehastim.set_pulse_amplitude(amplitudes=amplitudes)

    def set_stimulation_pulse_width(self, widths: int | list[int]):
        """Set the Rehastim stimulation pulse width.

        Parameters
        ----------
        widths : int | list[int]
            The width to set.
        """
        _logger.info("Setting Rehastim stimulation pulse width")
        self._rehastim.set_pulse_width(widths=widths)

    def set_stimulation_pulse_interval(self, interval: float):
        """Set the Rehastim stimulation interval.

        Parameters
        ----------
        interval : float
            The interval to set.
        """
        _logger.info("Setting Rehastim stimulation interval")
        self._rehastim.set_pulse_interval(interval=interval)

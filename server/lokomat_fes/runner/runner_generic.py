from abc import ABC, abstractmethod
import logging

from ..common.data import Data
from ..nidaq import NiDaqGeneric
from ..rehastim import RehastimGeneric
from ..scheduler.scheduler import Scheduler
from ..scheduler.stimulation import StimulationAbstract

logger = logging.getLogger("lokomat_fes")


class RunnerGeneric(ABC):
    """Abstract base class for Runners."""

    def __init__(self, rehastim: RehastimGeneric, nidaq: NiDaqGeneric) -> None:
        """Initialize the Runner."""
        logger.info("Initializing the Runner")

        self._rehastim = rehastim
        self._nidaq = nidaq

        self._data_for_scheduler = Data()
        self._nidaq.register_to_data_ready(self._data_for_scheduler.nidaq.add_sample_block)
        self._scheduler = Scheduler(runner=self, data=self._data_for_scheduler)

        self._trial_data = None
        self._is_recording = False

    def exec(self):
        """Start the Runner."""
        logger.info("Starting the Runner")
        self._exec()

        # Make sure the devices are stopped
        if self._is_recording:
            self.stop_recording()

        self._scheduler.dispose()

    @abstractmethod
    def _exec(self) -> None:
        """Start the Runner (implementation)."""

    ### DATA RELATED METHODS ###
    def save_trial(self, filename: str) -> None:
        """Save the last recorded trial.

        Parameters
        ----------
        filename : str
            The filename to save the trial to.
        """
        if self._trial_data is None:
            raise RuntimeError("No trial recorded")
        if self._is_recording:
            raise RuntimeError("Cannot save while recording")

        logger.info("Saving the last recorded trial")
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

    def _prepare_trial(self):
        """Prepare the data."""
        logger.info("Preparing the data")
        self._trial_data = Data()

        # Initialize the callback to record the data
        self._nidaq.register_to_data_ready(self._trial_data.nidaq.add_sample_block)
        self._rehastim.register_to_on_stimulation_started(self._trial_data.rehastim.add)
        self._rehastim.register_to_on_stimulation_stopped(self._trial_data.rehastim.stop_undefined_stimulation_duration)

    def _finalize_trial(self):
        """Finalize the data."""
        logger.info("Finalizing the data")
        self._nidaq.unregister_to_data_ready(self._trial_data.nidaq.add_sample_block)
        self._rehastim.unregister_to_on_stimulation_started(self._trial_data.rehastim.add)
        self._rehastim.unregister_to_on_stimulation_stopped(
            self._trial_data.rehastim.stop_undefined_stimulation_duration
        )

    ### KINEMATIC DEVICE (NIDAQ) RELATED METHODS ###
    def start_recording(self):
        if self._is_recording:
            raise RuntimeError("Already recording")

        """Start the recording."""
        logger.info("Starting recording")
        self._prepare_trial()
        self._nidaq.start_recording()
        self._is_recording = True

    def stop_recording(self):
        """Stop the recording."""
        if not self._is_recording:
            raise RuntimeError("Not recording")

        logger.info("Stopping recording")
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
        logger.info("Scheduling a stimulation")
        self._scheduler.add(stimulation=stimulation)

    def get_scheduled_stimulations(self) -> list[StimulationAbstract]:
        """Get the scheduled stimulations.

        Returns
        -------
        list[StimulationAbstract]
            The scheduled stimulations.
        """
        logger.info("Getting the scheduled stimulations")
        return self._scheduler.get_stimulations()

    def remove_scheduled_stimulation(self, index: int):
        """Remove a scheduled stimulation. To list the currently scheduled stimulations, use get_scheduled_stimulations().

        Parameters
        ----------
        index : int
            The index of the stimulation to remove.
        """
        logger.info("Removing a scheduled stimulation")
        self._scheduler.remove(stimulation=self._scheduler.get_stimulations()[index])

    def start_stimulation(self, duration: float):
        """Start the Rehastim stimulation."""
        logger.info("Starting Rehastim stimulation")
        self._rehastim.start_stimulation(duration)

    def stop_stimulation(self):
        """Stop the Rehastim stimulation."""
        logger.info("Stopping Rehastim stimulation")
        self._rehastim.stop_stimulation()

    def set_stimulation_pulse_amplitude(self, amplitudes: float | list[float]):
        """Set the Rehastim stimulation amplitude.

        Parameters
        ----------
        amplitudes : float | list[float]
            The amplitude to set. If a list is provided, the amplitude is set for each channel.
        """
        logger.info("Setting Rehastim stimulation amplitude")
        self._rehastim.set_pulse_amplitude(amplitudes=amplitudes)

    def set_stimulation_pulse_width(self, widths: int | list[int]):
        """Set the Rehastim stimulation pulse width.

        Parameters
        ----------
        widths : int | list[int]
            The width to set.
        """
        logger.info("Setting Rehastim stimulation pulse width")
        self._rehastim.set_pulse_width(widths=widths)

    def set_stimulation_pulse_interval(self, interval: float):
        """Set the Rehastim stimulation interval.

        Parameters
        ----------
        interval : float
            The interval to set.
        """
        logger.info("Setting Rehastim stimulation interval")
        self._rehastim.set_pulse_interval(interval=interval)

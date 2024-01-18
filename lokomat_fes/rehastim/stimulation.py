from abc import ABC, abstractmethod


class StimulationAbstract(ABC):
    @abstractmethod
    def should_stimulate(self, time: float, stride_point: float) -> bool:
        """Check whether to stimulate at a given time.
        This method is called at every iteration of the main loop.
        As long as this method returns False, nothing happens. As soon as it returns True, the stimulation is performed.
        The method is therefore responsible for deciding when the stimulation should stop, which is done by returning
        False again. This means that some kind of internal state needs to be kept track of. The method can assume that
        actual stimulation is starts as soon as it starts returning True and stops as soon as it starts returning False.

        Parameters
        ----------
        time : float
            The global time in seconds.
        stride_point : float
            The current percentage of the stride we are [0; 1].

        Returns
        -------
        bool
            Whether to stimulate or not.
        """


class StrideBasedStimulation(StimulationAbstract):
    def __init__(self, starting_point: float, end_point: float) -> None:
        """
        Parameters
        ----------
        starting_point : float
            The starting point of the stimulation as a percentage of the stride [0; 1].
        end_point : float
            The end point of the stimulation as a percentage of the stride [0; 1].
        """
        super(StrideBasedStimulation, self).__init__()
        self._starting_point = starting_point
        self._end_point = end_point

        self._is_stimulating = False

    def should_stimulate(self, time: float, stride_point: float) -> bool:
        if self._is_stimulating:
            if self._end_point < self._starting_point:
                # If the end point is "before" the starting point, it means it is earlier in the stride cycle, but
                # obviously after the starting point. This means we need to adjust the current stride point to be one
                # cycle before, so that the comparison works correctly.
                stride_point -= 1
            if stride_point >= self._end_point:
                self._is_stimulating = False

        else:
            if self._starting_point >= stride_point:
                self._is_stimulating = True

        return self._is_stimulating


class TimeBasedStimulation(StimulationAbstract):
    def __init__(self, start_point: float, end_time: float) -> None:
        """
        Parameters
        ----------
        starting_point : float
            The starting point of the stimulation as a percentage of the stride [0; 1].
        end_time : float
            The end time of the stimulation in seconds.
        """
        super(TimeBasedStimulation, self).__init__()
        self._starting_point = start_point
        self._end_time = end_time

        self._stimulation_started_at = None

    @property
    def _is_stimulating(self):
        return self._stimulation_started_at is not None

    def should_stimulate(self, time: float, stride_point: float) -> bool:
        if self._is_stimulating:
            if time >= self._stimulation_started_at + self._end_time:
                self._stimulation_started_at = None

        else:
            if self._starting_point >= stride_point:
                self._stimulation_started_at = time

        return self._is_stimulating

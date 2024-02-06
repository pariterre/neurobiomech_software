from functools import partial
from operator import gt, ge, lt, le
from typing import Callable


from ..common.data import Data
from .data_analyser import Side, DataAnalyser, GaitEvent


class AutomaticStimulationRule:
    def __init__(
        self,
        name: str,
        channels: list[int],
        amplitudes: list[float],
        start_stimulating_rule: Callable[[float, float, float], tuple[bool, ...]] = None,
        continue_stimulating_rule: Callable[[float, float, float], tuple[bool, ...]] = None,
        end_stimulating_rule: Callable[[float, float, float], tuple[bool, ...]] = None,
    ) -> None:
        """Initialize the stimulation.

        Parameters
        ----------
        name : str
            The name of the stimulation.
        condition_function: Callable[[float, float, float], tuple[bool, ...]]
            A function that takes the current time, stride percentage on left and right [0; 1]
            and returns a tuple of is_stimulating for each channels.
        """
        self._name = name
        self._channels = channels
        self._amplitudes = amplitudes
        self._started_stimulating_at: float | None = None

        if start_stimulating_rule is None:
            raise ValueError("start_stimulating_rule cannot be None")
        self._start_stimulating_rule = start_stimulating_rule

        if continue_stimulating_rule is None and end_stimulating_rule is not None:
            raise ValueError(
                "At least one stopping rule is defined (either continue_stimulating_function or end_stimulating_function)"
            )
        self._continue_stimulating_rule = continue_stimulating_rule
        self._end_stimulating_rule = end_stimulating_rule

    @property
    def name(self) -> str:
        """Get the name of the stimulation."""
        return self._name

    def stimulation_amplitudes(self, current_time: float, data: Data, amplitude_out: list[float]) -> None:
        """Check whether to stimulate at a given time.
        This method is called at every milliseconds.

        As long as this method returns None for a specific channel, nothing happens on this channel (meaning if it was
        stimulating, it keeps stimulating and if it was not stimulating, it keeps not stimulating).
        If it returns a positive value, the stimulation is should start for this channel for the given duration in
        seconds. If it returns a negative value, the stimulation is should stop for this channel. If a 0 is sent, a
        stimulation is started, but should not be automatically stopped (until a negative value is sent).

        Parameters
        ----------
        current_time : float
            The current time in seconds since t0.
        data : Data
            The current data
        amplitude_out : list[float | None]
            The amplitude to stimulate for each channel. If None, the channel should not change its state.

        Returns
        -------
        bool
            Whether to stimulate or not.
        """
        # Get where we are in a stride cycle and whether we should stimulate or not channels
        stride_left = DataAnalyser.percentage_of_stride(data, Side.LEFT)
        stride_right = DataAnalyser.percentage_of_stride(data, Side.RIGHT)
        if stride_left < 0 or stride_right < 0:
            return

        time_since_last_stim = (
            current_time - self._started_stimulating_at if self._started_stimulating_at is not None else None
        )
        start = self._start_stimulating_rule(time_since_last_stim, stride_left, stride_right)
        if self._continue_stimulating_rule is not None:
            stop = not start or not self._continue_stimulating_rule(time_since_last_stim, stride_left, stride_right)
        else:
            stop = self._end_stimulating_rule(time_since_last_stim, stride_left, stride_right)

        if self._started_stimulating_at is not None and stop:
            # We don't mind if start is set as if we get to stop we will stop anyway
            self._started_stimulating_at = None
            for index in range(len(amplitude_out)):
                amplitude_out[index] = 0
            return
        elif self._started_stimulating_at is None and start:
            self._started_stimulating_at = current_time
            for amplitude_index, i in enumerate(self._channels):
                amplitude_out[i] = self._amplitudes[amplitude_index]
            return

    def __str__(self) -> str:
        return self.name

    @classmethod
    def from_json(cls, data: dict) -> "AutomaticStimulationRule":
        """Create a StimulationCondition from a json dictionary."""

        if "name" not in data:
            raise ValueError("name must be defined")
        name = data["name"]

        if "pulse" not in data:
            raise ValueError("pulse must be defined")
        pulse = data["pulse"]
        if "channels" not in pulse:
            raise ValueError("channels must be defined in pulse")
        channels = pulse["channels"]
        if "amplitudes" not in pulse:
            raise ValueError("amplitudes must be defined in pulse")
        amplitudes = pulse["amplitudes"]

        start_stimulating_rule = None
        if "start_stimulating_rule" in data:
            start_stimulating_rule = _condition_from_json(data["start_stimulating_rule"])

        continue_stimulating_rule = None
        if "continue_stimulating_rule" in data:
            continue_stimulating_rule = _condition_from_json(data["continue_stimulating_rule"])

        end_stimulating_rule = None
        if "end_stimulating_rule" in data:
            end_stimulating_rule = _condition_from_json(data["end_stimulating_rule"])

        return cls(
            name=name,
            channels=channels,
            amplitudes=amplitudes,
            start_stimulating_rule=start_stimulating_rule,
            continue_stimulating_rule=continue_stimulating_rule,
            end_stimulating_rule=end_stimulating_rule,
        )


def _condition_from_json(data: dict) -> Callable[[float, float, float], bool]:
    """
    TODO
    """

    gait_percentage = None
    duration = None
    if "gait_event" in data:
        if data["gait_event"] == "heel_strike_0":
            gait_percentage = GaitEvent.HEEL_STRIKE_0.value
        elif data["gait_event"] == "toe_off":
            gait_percentage = GaitEvent.TOE_OFF.value
        elif data["gait_event"] == "heel_strike_100":
            gait_percentage = GaitEvent.HEEL_STRIKE_100.value
        else:
            raise ValueError("gait_event must be either 'heel_strike_0', 'heel_strike_100' or 'toe_off'")
    elif "gait_percentage" in data:
        gait_percentage = data["gait_percentage"]

    if "duration" in data:
        duration = data["duration"]

    if gait_percentage is None and duration is None:
        raise ValueError(
            "gait_percentage or duration must be defined. If both are defined, the first to occur will be used."
        )

    side = None
    if "side" in data:
        if data["side"] == "left":
            side = Side.LEFT
        elif data["side"] == "right":
            side = Side.RIGHT
        else:
            raise ValueError("side must be either 'left' or 'right'")
    else:
        if gait_percentage is not None:
            raise ValueError("side must be defined if gait_percentage is defined")

    if data["comparison"] in ("greater_or_equal", ">=", "from"):
        comparison = ge
    elif data["comparison"] in ("greater_than", ">", "after"):
        comparison = gt
    elif data["comparison"] in ("less_or_equal", "<=", "to"):
        comparison = le
    elif data["comparison"] in ("less_than", "<", "before"):
        comparison = lt
    else:
        raise ValueError(
            "comparison must be either\n"
            "\t'greater_or_equal', '>=' or 'from' for greater or equal\n"
            "\t'greater', '>' or 'after' for greater\n"
            "\t'less_or_equal', '<=' or 'to' for less or equal\n"
            "\t'less', '<' or 'before' for less"
        )

    return partial(
        _should_stimulate,
        side=side,
        duration=duration,
        gait_percentage=gait_percentage,
        comparison=comparison,
    )


def _should_stimulate(
    current_time: float | None,
    current_left_gait_percentage: float,
    current_right_gait_percentage: float,
    side: Side,
    duration: float | None,
    gait_percentage: float | None,
    comparison: Callable[[float, float], bool],
):
    """
    TODO
    """
    if gait_percentage is not None:
        if side == Side.LEFT:
            return comparison(current_left_gait_percentage, gait_percentage)
        elif side == Side.RIGHT:
            return comparison(current_right_gait_percentage, gait_percentage)
        else:
            raise ValueError("side must be either 'left' or 'right'")
    elif duration is not None:
        return current_time < duration if current_time is not None else False
    else:
        raise ValueError("gait_percentage or duration must be defined")

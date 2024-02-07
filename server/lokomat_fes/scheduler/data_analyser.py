from enum import Enum, auto

import numpy as np

from ..common.data import Data


class Side(Enum):
    LEFT = auto()
    RIGHT = auto()
    BOTH = auto()


class GaitEvent(Enum):
    """Gait events."""

    HEEL_STRIKE_0 = 0.0
    TOE_OFF = 0.6
    HEEL_STRIKE_100 = 1.0


class DataAnalyser:
    @staticmethod
    def percentage_of_stride(data: Data, side: Side) -> float:
        """Get the percentage of the stride cycle.

        Parameters
        ----------
        data : Data
            The data to use.
        side : Side
            The side to use.

        Returns
        -------
        float
            The percentage of the stride cycle [0; 1].
        """

        if len(data.nidaq) < 2:
            return -1  # Not enough data

        # Get the last two samples
        _, previous_data = data.nidaq.sample_block(index=-2, unsafe=True)
        _, current_data = data.nidaq.sample_block(index=-1, unsafe=True)
        if previous_data is None or current_data is None:
            return -1  # Not enough data

        previous_hip = np.mean(previous_data[0])
        current_hip = np.mean(current_data[0])
        if side == Side.RIGHT:
            # Simulate the right side by inverting the sign of the hip channel
            previous_hip = -previous_hip
            current_hip = -current_hip

        # The first channel is a sin wave, so
        #   if the value is [0; 1] and is increasing, we are in the first quarter of the stride cycle
        #   if the value is [0; 1] and is decreasing, we are in the second quarter of the stride cycle
        #   if the value is [0; -1] and is decreasing, we are in the third quarter of the stride cycle
        #   if the value is [0; -1] and is increasing, we are in the fourth quarter of the stride cycle
        # We return a linear interpolation of the percentage of the stride cycle we are in
        if current_hip >= 0 and current_hip > previous_hip:
            return 0 + 0.25 * current_hip
        elif current_hip >= 0 and current_hip < previous_hip:
            return 0.25 + 0.25 * (1 - current_hip)
        elif current_hip < 0 and current_hip < previous_hip:
            return 0.5 + 0.25 * np.abs(previous_hip)
        elif current_hip < 0 and current_hip > previous_hip:
            return 0.75 + 0.25 * (1 - np.abs(previous_hip))
        else:
            return -1  # Error

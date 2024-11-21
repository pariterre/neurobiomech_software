part of 'time_series_data.dart';

class EmgTimeSeriesData extends TimeSeriesData {
  EmgTimeSeriesData({
    required super.initialTime,
    required super.channelCount,
    required super.isFromLiveData,
    int slidingRmsWindowSize = 50,
  })  : _dataBackup = List.generate(channelCount, (_) => <double>[]),
        _slidingRmsWindowSize = slidingRmsWindowSize;

  int _slidingRmsWindowSize;
  final List<List<double>> _dataBackup;

  @override
  void clear() {
    super.clear();

    for (var channel in _dataBackup) {
      channel.clear();
    }
  }

  int get slidingRmsWindowSize => _slidingRmsWindowSize;
  set slidingRmsWindowSize(int value) {
    if (value < 0) return;
    _slidingRmsWindowSize = value;
  }

  @override
  int dropBefore(double elapsedTime) {
    final firstIndexToKeep = super.dropBefore(elapsedTime);
    if (firstIndexToKeep <= 0) return firstIndexToKeep;

    for (var channel in _dataBackup) {
      channel.removeRange(0, firstIndexToKeep);
    }
    return firstIndexToKeep;
  }

  @override
  int appendFromJson(Map<String, dynamic> json) {
    final firstNewIndex = super.appendFromJson(json);
    if (firstNewIndex == -1) return -1;
    final lastNewIndex = time.length;

    // Backup the new data to the backup data
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      for (int i = firstNewIndex; i < lastNewIndex; i++) {
        _dataBackup[channelIndex].add(data[channelIndex][i]);
      }
    }
    if (slidingRmsWindowSize == 0) return firstNewIndex;

    // Perform an RMS on the new data, we start earlier to account for any non-computed
    // data yet, with the expand of redoing some computation
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      for (int start = firstNewIndex - 2 * slidingRmsWindowSize;
          start < lastNewIndex - slidingRmsWindowSize;
          start++) {
        if (start < 0) continue;

        // Determine the range for this chunk, and compute the RMS
        final end = start + slidingRmsWindowSize;
        data[channelIndex][start] =
            _computeRms(_dataBackup[channelIndex].sublist(start, end));
      }
      for (int start = lastNewIndex - slidingRmsWindowSize;
          start < lastNewIndex;
          start++) {
        data[channelIndex][start] = 0;
      }
    }

    return firstNewIndex;
  }
}

double _computeRms(List<double> data) {
  return sqrt(data.fold<double>(
          0, (previousValue, element) => previousValue + element * element) /
      data.length);
}

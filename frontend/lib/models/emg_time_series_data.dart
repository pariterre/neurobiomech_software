part of 'time_series_data.dart';

class EmgTimeSeriesData extends TimeSeriesData {
  EmgTimeSeriesData({
    required super.initialTime,
    required super.channelCount,
    required super.isFromLiveData,
    int defaultSlidingRmsWindow = 50,
  })  : _dataRaw = List.generate(channelCount, (_) => <double>[]),
        _slidingRmsWindows =
            List<int>.filled(channelCount, defaultSlidingRmsWindow);

  final List<int> _slidingRmsWindows;
  late final List<bool> _applySlidingRms = List.filled(channelCount, true);
  final List<List<double>> _dataRaw;

  @override
  List<List<double>> getData({bool raw = false}) => raw ? _dataRaw : _data;

  @override
  void clear() {
    super.clear();

    for (var channel in _dataRaw) {
      channel.clear();
    }
  }

  void setSlidingRmsWindow(int value, {required int channel}) {
    if (value < 0) return;
    _slidingRmsWindows[channel] = value;
  }

  int getSlidingRmsWindow({required int channel}) =>
      _slidingRmsWindows[channel];

  void setApplySlidingRms(bool value, {required int channel}) {
    _applySlidingRms[channel] = value;
  }

  bool getApplySlidingRms({required int channel}) => _applySlidingRms[channel];

  @override
  int dropBefore(double elapsedTime) {
    final firstIndexToKeep = super.dropBefore(elapsedTime);
    if (firstIndexToKeep <= 0) return firstIndexToKeep;

    for (var channel in _dataRaw) {
      channel.removeRange(0, firstIndexToKeep);
    }
    return firstIndexToKeep;
  }

  @override
  int appendFromJson(Map<String, dynamic> json) {
    final firstNewIndex = super.appendFromJson(json);
    if (firstNewIndex == -1) return -1;
    if (!isFromLiveData) return firstNewIndex;

    final lastNewIndex = time.length;

    // Backup the new data to the backup data
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      for (int i = firstNewIndex; i < lastNewIndex; i++) {
        _dataRaw[channelIndex].add(_data[channelIndex][i]);
      }
    }
    if (_slidingRmsWindows.indexWhere((e) => e != 0) < 0) return firstNewIndex;
    if (_applySlidingRms.indexWhere((e) => e) < 0) return firstNewIndex;

    // Perform an RMS on the new data, we start earlier to account for any non-computed
    // data yet, with the expand of redoing some computation
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      if (!_applySlidingRms[channelIndex]) continue;

      for (int start = firstNewIndex - 2 * _slidingRmsWindows[channelIndex];
          start < lastNewIndex - _slidingRmsWindows[channelIndex];
          start++) {
        if (start < 0) continue;

        // Determine the range for this chunk, and compute the RMS
        final end = start + _slidingRmsWindows[channelIndex];
        _data[channelIndex][start] =
            _computeRms(_dataRaw[channelIndex].sublist(start, end));
      }
      for (int start = lastNewIndex - _slidingRmsWindows[channelIndex];
          start < lastNewIndex;
          start++) {
        _data[channelIndex][start] = 0;
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

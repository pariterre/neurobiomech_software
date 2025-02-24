part of 'time_series_data.dart';

class PredictionData extends TimeSeriesData {
  PredictionData({
    required super.initialTime,
    required super.channelCount,
  }) : super(isFromLiveData: true);

  final List<String> labels = [];

  @override
  void clear({bool resetInternal = false}) {
    super.clear();
    if (resetInternal) {
      labels.clear();
      _data.clear();
    }
  }

  void addPrediction(String label) {
    labels.add(label);
    _channelCount++;

    _data.add(List.filled(time.length, 0.0, growable: true));
  }

  void removePrediction(String label) {
    final index = labels.indexOf(label);
    if (index == -1) return;

    labels.removeAt(index);
    _channelCount--;

    _data.removeAt(index);
  }

  @override
  int appendFromJson(Map<String, dynamic> json) {
    final entries = (json['data'] as Map<String, dynamic>).entries;
    for (int i = 0; i < entries.length; i++) {
      final data = entries.elementAt(i);

      if (i == 0) {
        // If this is the first time stamps, we need to set the time offset
        _timeOffset ??= (data.value[0] as int) / 1000.0;
        time.add((data.value[0] as int) / 1000.0 - _timeOffset!);
      }
      _data[i].addAll(
          (data.value[1] as List<dynamic>).map((e) => e as double).toList());
    }

    return time.length;
  }
}

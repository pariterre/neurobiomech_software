part of 'time_series_data.dart';

class PredictionData extends TimeSeriesData {
  PredictionData({
    required super.initialTime,
    required super.channelCount,
  }) : super(isFromLiveData: true);

  final List<String> labels = [];

  void addPrediction(String label) {
    labels.add(label);
    _channelCount++;

    _data.add(List.filled(time.length, 0.0));
  }

  void removePrediction(String label) {
    final index = labels.indexOf(label);
    if (index == -1) return;

    labels.removeAt(index);
    _channelCount--;

    _data.removeAt(index);
  }
}

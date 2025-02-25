import 'dart:io';
import 'dart:math';
part 'emg_time_series_data.dart';
part 'prediction_data.dart';

class TimeSeriesData {
  bool isFromLiveData;
  double? _timeOffset; // How many milliseconds to subtract to the time vector
  int _channelCount;
  int get channelCount => _channelCount;

  final List<double> time = []; // In milliseconds since t0
  final List<List<double>> _data;
  List<List<double>> getData({bool raw = false}) => _data;

  void clear() {
    resetTime();
    for (var channel in _data) {
      channel.clear();
    }
  }

  void resetTime() {
    time.clear();
    _timeOffset = null;
  }

  int get length => time.length;
  bool get isEmpty => time.isEmpty;
  bool get isNotEmpty => time.isNotEmpty;

  TimeSeriesData({
    required DateTime initialTime,
    required int channelCount,
    required this.isFromLiveData,
  })  : _channelCount = channelCount,
        _data = List.generate(channelCount, (_) => <double>[]);

  int appendFromJson(Map<String, dynamic> json) {
    final timeSeries = (json['data'] as List<dynamic>);

    // If this is the first time stamps, we need to set the time offset
    if (timeSeries.isEmpty) return -1;
    bool isNew = _timeOffset == null;
    _timeOffset ??=
        ((isFromLiveData ? timeSeries.last[0] : timeSeries.first[0]) as int) /
            1000.0;

    final maxLength = timeSeries.length;
    final newT =
        timeSeries.map((e) => (e[0] as int) / 1000.0 - _timeOffset!).toList();

    // Find the first index where the new time is larger than the last time of t
    final firstNewIndex =
        isNew ? 0 : newT.indexWhere((value) => value > time.last);
    time.addAll(newT.getRange(firstNewIndex, maxLength));

    // Parse the data for each channel
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      _data[channelIndex].addAll(timeSeries
          .getRange(firstNewIndex, maxLength)
          .map<double>((e) => e[1][channelIndex]));
    }

    return time.length - (maxLength - firstNewIndex);
  }

  Future<void> toFile(String path, {bool raw = false}) async {
    final file = File(path);
    final sink = file.openWrite();
    sink.writeln(
        'time (s),${List.generate(channelCount, (index) => 'channel$index').join(',')}');

    final data = getData(raw: raw);
    for (int i = 0; i < time.length; i++) {
      sink.write((time[i] / 1000).toStringAsFixed(4));
      for (int j = 0; j < channelCount; j++) {
        sink.write(',');
        sink.write(data[j][i].toStringAsFixed(6));
      }
      sink.writeln();
    }
    await sink.flush();
    await sink.close();
  }

  int dropBefore(double elapsedTime) {
    if (time.isEmpty) return 0;

    final firstIndexToKeep = time.indexWhere((value) => value >= elapsedTime);
    if (firstIndexToKeep == -1) {
      // If we get to the end, we should drop everything
      clear();
    } else {
      time.removeRange(0, firstIndexToKeep);
      for (var channel in _data) {
        channel.removeRange(0, firstIndexToKeep);
      }
    }
    return firstIndexToKeep;
  }
}

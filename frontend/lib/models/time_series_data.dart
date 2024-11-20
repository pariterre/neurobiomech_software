import 'dart:io';

class TimeSeriesData {
  bool isFromLiveData;
  double? _timeOffset; // How many milliseconds to subtract to the time vector
  final int channelCount;

  final List<double> time = []; // In milliseconds since t0
  final List<List<double>> data;

  void clear() {
    time.clear();
    _timeOffset = null;

    for (var channel in data) {
      channel.clear();
    }
  }

  int get length => time.length;
  bool get isEmpty => time.isEmpty;
  bool get isNotEmpty => time.isNotEmpty;

  TimeSeriesData({
    required DateTime initialTime,
    required this.channelCount,
    required this.isFromLiveData,
  }) : data = List.generate(channelCount, (_) => <double>[]);

  appendFromJson(Map<String, dynamic> json) {
    final timeSeries = (json['data'] as List<dynamic>);

    // If this is the first time stamps, we need to set the time offset
    _timeOffset ??=
        ((isFromLiveData ? timeSeries.last[0] : timeSeries.first[0]) as int) /
            1000.0;

    final maxLength = timeSeries.length;
    final newT =
        timeSeries.map((e) => (e[0] as int) / 1000.0 - _timeOffset!).toList();

    // Find the first index where the new time is larger than the last time of t
    final firstTIndex =
        time.isEmpty ? 0 : newT.indexWhere((value) => value > time.last);
    time.addAll(newT.getRange(firstTIndex, maxLength));

    // Parse the data for each channel
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      data[channelIndex].addAll(timeSeries
          .getRange(firstTIndex, maxLength)
          .map<double>((e) => e[1][channelIndex]));
    }
  }

  Future<void> toFile(String path) async {
    final file = File(path);
    final sink = file.openWrite();
    sink.writeln(
        'time (s),${List.generate(channelCount, (index) => 'channel$index').join(',')}');
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

  void dropBefore(double elapsedTime) {
    if (time.isEmpty) return;

    final firstIndexToKeep = time.indexWhere((value) => value >= elapsedTime);
    if (firstIndexToKeep == -1) {
      // If we get to the end, we should drop everything
      clear();
    } else {
      time.removeRange(0, firstIndexToKeep);
      for (var channel in data) {
        channel.removeRange(0, firstIndexToKeep);
      }
    }
  }
}

class TimeSeriesData {
  DateTime _initialTime;
  DateTime get initialTime => _initialTime;
  double? _timeOffset; // How many milliseconds to subtract to the time vector
  final int channelCount;

  final List<double> time = []; // In milliseconds since t0
  final List<List<double>> data;

  void clear({DateTime? initialTime}) {
    _initialTime = initialTime ?? _initialTime;
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
  })  : _initialTime = initialTime,
        data = List.generate(channelCount, (_) => <double>[]);

  appendFromJson(Map<String, dynamic> json) {
    final timeSeries = (json['data'] as List<dynamic>);

    // If this is the first time stamps, we need to set the time offset
    _timeOffset ??= (timeSeries[0][0] as int) / 1000.0;

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

  void dropBefore(double elapsedTime) {
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

class TimeSeriesData {
  final double t0;
  final int channelCount;

  final List<double> t = [];
  final List<List<double>> data;

  void clear() {
    t.clear();
    for (var channel in data) {
      channel.clear();
    }
  }

  int get length => t.length;
  bool get isEmpty => t.isEmpty;
  bool get isNotEmpty => t.isNotEmpty;

  TimeSeriesData({
    required this.t0,
    required this.channelCount,
  }) : data = List.generate(channelCount, (_) => <double>[]);

  appendFromJson(Map<String, dynamic> json) {
    final timeSeries = (json['data'] as List<dynamic>);

    // From microseconds to seconds
    final maxLength = timeSeries.length;
    final newT =
        timeSeries.map((e) => (e[0] as int) / 1000.0 / 1000.0).toList();

    // Find the first index where the new time is larger than the last time of t
    final firstTIndex = newT.indexWhere((value) => value > t.last);
    t.addAll(newT.getRange(firstTIndex, maxLength));

    // Parse the data for each channel
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      data[channelIndex].addAll(timeSeries
          .getRange(firstTIndex, maxLength)
          .map<double>((e) => e[1][channelIndex]));
    }
  }
}

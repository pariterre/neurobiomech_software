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
    t.addAll(timeSeries.map((e) => (e[0] as int) / 1000.0 / 1000.0));

    // Parse the data for each channel
    for (int channelIndex = 0; channelIndex < channelCount; channelIndex++) {
      data[channelIndex]
          .addAll(timeSeries.map<double>((e) => e[1][channelIndex]));
    }
  }
}

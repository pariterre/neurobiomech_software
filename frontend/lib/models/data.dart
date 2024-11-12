import 'package:frontend/models/time_series_data.dart';

class Data {
  DateTime _initialTime;
  DateTime get initialTime => _initialTime;
  final TimeSeriesData delsysAnalog;
  final TimeSeriesData delsysEmg;

  void clear({DateTime? initialTime}) {
    _initialTime = initialTime ?? _initialTime;
    delsysAnalog.clear(initialTime: initialTime);
    delsysEmg.clear(initialTime: initialTime);
  }

  bool get isEmpty => delsysAnalog.isEmpty && delsysEmg.isEmpty;

  bool get isNotEmpty => !isEmpty;

  Data({
    required DateTime initialTime,
    required int analogChannelCount,
    required int emgChannelCount,
  })  : _initialTime = initialTime,
        delsysAnalog = TimeSeriesData(
            initialTime: initialTime, channelCount: analogChannelCount),
        delsysEmg = TimeSeriesData(
            initialTime: initialTime, channelCount: emgChannelCount);

  appendFromJson(List json) {
    for (Map data in json) {
      final deviceName = data['name'];
      final deviceData = data['data'] as Map<String, dynamic>;
      if (deviceName == 'DelsysAnalogDataCollector') {
        delsysAnalog.appendFromJson(deviceData);
      } else if (deviceName == 'DelsysEmgDataCollector') {
        delsysEmg.appendFromJson(deviceData);
      }
    }
  }

  void dropBefore(DateTime t) {
    delsysAnalog.dropBefore(
        (initialTime.millisecondsSinceEpoch - t.millisecondsSinceEpoch)
            .toDouble());
    delsysEmg.dropBefore(
        (initialTime.millisecondsSinceEpoch - t.millisecondsSinceEpoch)
            .toDouble());
  }
}

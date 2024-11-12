import 'package:frontend/models/time_series_data.dart';

class Data {
  final double t0;
  final TimeSeriesData delsysAnalog;
  final TimeSeriesData delsysEmg;

  void clear() {
    delsysAnalog.clear();
    delsysEmg.clear();
  }

  bool get isEmpty => delsysAnalog.isEmpty && delsysEmg.isEmpty;

  bool get isNotEmpty => !isEmpty;

  Data({
    required this.t0,
    required int analogChannelCount,
    required int emgChannelCount,
  })  : delsysAnalog = TimeSeriesData(t0: t0, channelCount: analogChannelCount),
        delsysEmg = TimeSeriesData(t0: t0, channelCount: emgChannelCount);

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
}

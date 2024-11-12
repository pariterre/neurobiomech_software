import 'package:frontend/models/time_series_data.dart';

class Data {
  double _t0;
  double get t0 => _t0;
  final TimeSeriesData delsysAnalog;
  final TimeSeriesData delsysEmg;

  void clear({double? t0}) {
    _t0 = t0 ?? _t0;
    delsysAnalog.clear();
    delsysEmg.clear();
  }

  bool get isEmpty => delsysAnalog.isEmpty && delsysEmg.isEmpty;

  bool get isNotEmpty => !isEmpty;

  Data({
    required double t0,
    required int analogChannelCount,
    required int emgChannelCount,
  })  : _t0 = t0,
        delsysAnalog = TimeSeriesData(t0: t0, channelCount: analogChannelCount),
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

  void dropBefore(double t) {
    delsysAnalog.dropBefore(t - t0);
    delsysEmg.dropBefore(t - t0);
  }
}

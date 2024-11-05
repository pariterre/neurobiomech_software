import 'package:frontend/models/time_series_data.dart';

class Data {
  final double t0;
  final TimeSeriesData delsysAnalog;
  final TimeSeriesData delsysEmg;

  void clear() {
    delsysAnalog.clear();
    delsysEmg.clear();
  }

  Data({
    required this.t0,
    required int analogChannelCount,
    required int emgChannelCount,
  })  : delsysAnalog = TimeSeriesData(t0: t0, channelCount: analogChannelCount),
        delsysEmg = TimeSeriesData(t0: t0, channelCount: emgChannelCount);
}

import 'package:frontend/models/time_series_data.dart';

class Data {
  DateTime _initialTime;
  DateTime get initialTime => _initialTime;
  final TimeSeriesData delsysAnalog;
  final EmgTimeSeriesData delsysEmg;

  void clear({DateTime? initialTime}) {
    _initialTime = initialTime ?? _initialTime;

    delsysAnalog.clear();
    delsysEmg.clear();
  }

  bool get isEmpty => delsysAnalog.isEmpty && delsysEmg.isEmpty;

  bool get isNotEmpty => !isEmpty;

  Data({
    required DateTime initialTime,
    required int analogChannelCount,
    required int emgChannelCount,
    required bool isFromLiveData,
  })  : _initialTime = initialTime,
        delsysAnalog = TimeSeriesData(
            initialTime: initialTime,
            channelCount: analogChannelCount,
            isFromLiveData: isFromLiveData),
        delsysEmg = EmgTimeSeriesData(
            initialTime: initialTime,
            channelCount: emgChannelCount,
            isFromLiveData: isFromLiveData);

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
        (t.millisecondsSinceEpoch - initialTime.millisecondsSinceEpoch)
            .toDouble());
    delsysEmg.dropBefore(
        (t.millisecondsSinceEpoch - initialTime.millisecondsSinceEpoch)
            .toDouble());
  }

  Future<void> toFile(String path) async {
    final analogPath = '$path/analog.csv';
    final emgPath = '$path/emg.csv';
    final analogFuture = delsysAnalog.toFile(analogPath);
    final emgFuture = delsysEmg.toFile(emgPath);
    await Future.wait([analogFuture, emgFuture]);
  }
}

import 'package:frontend/models/time_series_data.dart';

enum DataGenericTypes { analogs, predictions }

class Data {
  final DataGenericTypes dataGenericType;
  DateTime _initialTime;
  DateTime get initialTime => _initialTime;
  final TimeSeriesData delsysAnalog;
  final EmgTimeSeriesData delsysEmg;
  final PredictionData predictions;

  void clear({DateTime? initialTime}) {
    _initialTime = initialTime ?? _initialTime;

    switch (dataGenericType) {
      case DataGenericTypes.analogs:
        delsysAnalog.clear();
        delsysEmg.clear();
        break;
      case DataGenericTypes.predictions:
        predictions.clear();
        break;
    }
  }

  bool get isEmpty => switch (dataGenericType) {
        DataGenericTypes.analogs => _isAnalogsEmpty,
        DataGenericTypes.predictions => _isPredictionsEmpty
      };
  bool get isNotEmpty => !isEmpty;

  bool get _isAnalogsEmpty => delsysAnalog.isEmpty && delsysEmg.isEmpty;
  bool get _isPredictionsEmpty => predictions.isEmpty;

  Data({
    required this.dataGenericType,
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
            isFromLiveData: isFromLiveData),
        predictions = PredictionData(
          initialTime: initialTime,
          channelCount: 0,
        );

  void appendFromJson(Map<String, dynamic> json) {
    switch (dataGenericType) {
      case DataGenericTypes.analogs:
        _appendAnalogsDataFromJson(json);
        break;
      case DataGenericTypes.predictions:
        _appendPredictionFromJson(json);
        break;
    }
  }

  void _appendAnalogsDataFromJson(Map<String, dynamic> json) {
    for (final data in json.values) {
      final deviceName = data['name'];
      final deviceData = data['data'] as Map<String, dynamic>;
      if (deviceName == 'DelsysAnalogDataCollector') {
        delsysAnalog.appendFromJson(deviceData);
      } else if (deviceName == 'DelsysEmgDataCollector') {
        delsysEmg.appendFromJson(deviceData);
      }
    }
  }

  void _appendPredictionFromJson(Map<String, dynamic> json) =>
      predictions.appendFromJson(json);

  void dropBefore(DateTime t) {
    switch (dataGenericType) {
      case DataGenericTypes.analogs:
        delsysAnalog.dropBefore(
            (t.millisecondsSinceEpoch - initialTime.millisecondsSinceEpoch)
                .toDouble());
        delsysEmg.dropBefore(
            (t.millisecondsSinceEpoch - initialTime.millisecondsSinceEpoch)
                .toDouble());
        break;
      case DataGenericTypes.predictions:
        predictions.dropBefore(
            (t.millisecondsSinceEpoch - initialTime.millisecondsSinceEpoch)
                .toDouble());
        break;
    }
  }

  Future<void> toFile(String path) async {
    switch (dataGenericType) {
      case DataGenericTypes.analogs:
        final analogPath = '$path/analog.csv';
        final emgPath = '$path/emg.csv';
        final analogFuture = delsysAnalog.toFile(analogPath);
        final emgFuture = delsysEmg.toFile(emgPath);
        await Future.wait([analogFuture, emgFuture]);
        break;
      case DataGenericTypes.predictions:
        throw UnimplementedError();
    }
  }
}

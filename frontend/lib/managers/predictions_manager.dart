import 'dart:convert';

import 'package:frontend/models/prediction_model.dart';
import 'package:shared_preferences/shared_preferences.dart';

class PredictionsManager {
  static final PredictionsManager _instance = PredictionsManager._internal();
  static Future<PredictionsManager> get instance async {
    if (!_instance._isLoaded) {
      await _instance.load();
    }
    return _instance;
  }

  bool _isLoaded = false;
  bool _isLoading = false;

  final List<PredictionModel> _predictions = [];
  List<PredictionModel> get predictions => List.unmodifiable(_predictions);

  PredictionsManager._internal() {
    load();
  }

  ///
  /// Load the data from the disk
  Future<void> load() async {
    if (_isLoading) {
      while (!_isLoaded) {
        await Future.delayed(const Duration(milliseconds: 100));
      }
      return;
    }
    _isLoading = true;

    _predictions.clear();

    final preferences = await SharedPreferences.getInstance();
    final predictions = preferences.getStringList('predictions');
    if (predictions != null) {
      for (final prediction in predictions) {
        _predictions
            .add(PredictionModel.fromSerialized(jsonDecode(prediction)));
      }
    }

    _isLoaded = true;
    _isLoading = false;
  }

  ///
  /// Save the data to the disk
  Future<void> save(List<PredictionModel> predictions) async {
    _predictions.clear();
    _predictions.addAll(predictions);

    final preferences = await SharedPreferences.getInstance();
    await preferences.setStringList(
      'predictions',
      _predictions.map((e) => jsonEncode(e.serialize())).toList(),
    );
  }
}

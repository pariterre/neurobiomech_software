import 'dart:convert';

import 'package:frontend/models/prediction_model.dart';
import 'package:shared_preferences/shared_preferences.dart';

class PredictionsManager {
  static final PredictionsManager _instance = PredictionsManager._internal();
  static PredictionsManager get instance => _instance;

  bool _isInitialized = false;

  final List<PredictionModel> _predictions = [];
  List<PredictionModel> get predictions {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    return List.unmodifiable(_predictions);
  }

  PredictionsManager._internal();

  ///
  /// Load the data from the disk
  Future<void> initialize() async {
    if (_isInitialized) return;

    _predictions.clear();

    final preferences = await SharedPreferences.getInstance();
    final predictions = preferences.getStringList('predictions');
    if (predictions != null) {
      for (final prediction in predictions) {
        _predictions
            .add(PredictionModel.fromSerialized(jsonDecode(prediction)));
      }
    }

    _isInitialized = true;
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

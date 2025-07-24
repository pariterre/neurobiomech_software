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

  final List<PredictionModel> _active = [];
  List<PredictionModel> get active {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    return List.unmodifiable(_active);
  }

  void clearActive() {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    _active.clear();
  }

  bool isActive(PredictionModel prediction) {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    for (final activePrediction in _active) {
      if (prediction == activePrediction) return true;
    }
    return false;
  }

  void addActive(PredictionModel prediction) {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    if (!_active.contains(prediction)) {
      _active.add(prediction);
    }
  }

  void removeActive(PredictionModel prediction) {
    if (!_isInitialized) {
      throw StateError('PredictionsManager is not initialized');
    }

    _active.remove(prediction);
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
      for (final predictionAsJson in predictions) {
        final prediction =
            PredictionModel.fromSerialized(jsonDecode(predictionAsJson));
        mergePrediction(prediction);
      }
    }

    _isInitialized = true;
  }

  void mergePrediction(PredictionModel newPrediction) {
    for (final prediction in _predictions) {
      // If the prediction already exists, we do not add it again
      if (prediction == newPrediction) return;
    }
    _predictions.add(newPrediction);
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

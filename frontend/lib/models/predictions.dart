class Predictions {
  final List<double> _predictions = [];

  List<double> get predictions => List.unmodifiable(_predictions);

  void clear() {
    _predictions.clear();
  }
}

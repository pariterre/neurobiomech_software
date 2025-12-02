import 'dart:async';

import 'package:logging/logging.dart';

final _logger = Logger('GenericListener');

///
/// To specify a specific type of listener, you can instantiate this class
/// as such: `final myListener = GenericListener<Function(String param1, bool param2)>();`
class GenericListener<T extends Function> {
  // Define a mutex to prevent adding/removing listeners while notifying
  Completer<void>? _notifying;

  ///
  /// List of active listeners to notify.
  final List<T> _listeners = [];

  Future<void> _waitWhileNotifying() async {
    if (_notifying == null) return;
    await _notifying!.future;
  }

  ///
  /// Start listening.
  Future<void> addListener(T listener) async {
    await _waitWhileNotifying();
    _listeners.add(listener);
  }

  ///
  /// Stop listening.
  Future<void> removeListener(T listener) async {
    await _waitWhileNotifying();
    _listeners.remove(listener);
  }

  ///
  /// Stop all listeners.
  Future<void> cancelListeners() async {
    await _waitWhileNotifying();
    _listeners.clear();
  }

  ///
  /// Notify all listeners.
  Future<void> notifyListeners(void Function(T listener) invoke) async {
    await _waitWhileNotifying();
    _notifying = Completer<void>();
    for (final listener in List<T>.from(_listeners)) {
      try {
        invoke(listener);
      } catch (err, st) {
        _logger.severe('Error notifying listener', err, st);
      }
    }
    _notifying?.complete();
    _notifying = null;
  }

  int get length => _listeners.length;
}

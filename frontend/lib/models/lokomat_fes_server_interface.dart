import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:frontend/models/commands.dart';
import 'package:logging/logging.dart';

class LokomatFesServerInterface {
  /// PUBLIC API ///

  ///
  /// Get the singleton instance of the TcpCommunication class.
  static LokomatFesServerInterface get instance => _instance;

  ///
  /// If the communication initialized.
  bool get isInitialized => _socket != null;
  bool get isRecording => _isRecording;
  bool get hasRecorded => _hasRecorded;

  ///
  /// Initialize the communication with the server. If the connection fails,
  /// the function will retry until it succeeds or the number of retries is
  /// reached.
  Future<void> initialize({
    String serverIp = 'localhost',
    int serverPort = 4042,
    int? nbOfRetries,
  }) async {
    if (isInitialized) return;

    while (nbOfRetries == null || nbOfRetries > 0) {
      try {
        _socket = await Socket.connect(serverIp, serverPort);
        _socket!.listen(_listenToAcknowledge);

        return;
      } on SocketException {
        final log = Logger('TcpCommunication');
        log.info('Connection failed, retrying...');
        await Future.delayed(const Duration(seconds: 1));
        nbOfRetries = nbOfRetries == null ? null : nbOfRetries - 1;
      }
    }
  }

  ///
  /// Send a command to the server. If the command requires parameters, they
  /// can be passed as a list of strings.
  /// Returns true if the command was successfully sent the connexion to
  /// the server is still alive, false otherwise.
  /// If the command was "shutdown" or "quit", the connection is closed. By its
  /// nature, the "shutdown" or "quit" commands will always return false.
  Future<bool> send(Commands command, [List<String>? parameters]) async {
    if (!isInitialized) {
      log.severe('Communication not initialized');
      return false;
    }

    // Construct and send the command
    _lastAcknowledge = null;

    String message = "${command.toInt()}:";
    if (parameters != null) {
      message += parameters.join(',');
    }
    _socket!.write(message);
    try {
      await _socket!.flush();
    } on SocketException {
      log.info('Connexion was closed by the server');
      return false;
    }
    log.info('Sent command: $command');

    if (command == Commands.shutdown || command == Commands.quit) {
      _dispose();
      return false;
    }

    // Receive acknowledgment from the server
    while (_lastAcknowledge == null) {
      await Future.delayed(const Duration(milliseconds: 100));
    }

    if (command == Commands.startRecording) {
      _isRecording = true;
      _hasRecorded = true;
    } else if (command == Commands.stopRecording) {
      _isRecording = false;
    }

    return _lastAcknowledge == "OK";
  }

  ///
  /// Close the connection to the server.
  void _dispose() {
    _socket?.close();
    _isRecording = false;
    _hasRecorded = false;
    _socket = null;
    log.info('Connection closed');
  }

  /// PRIVATE API ///
  Socket? _socket;
  String? _lastAcknowledge;

  bool _isRecording = false;
  bool _hasRecorded = false;

  final log = Logger('TcpCommunication');

  ///
  /// Listen to the server's acknowledgment.
  void _listenToAcknowledge(List<int> data) =>
      _lastAcknowledge = utf8.decode(data);

  // Prepare the singleton
  static final LokomatFesServerInterface _instance =
      LokomatFesServerInterface._();
  LokomatFesServerInterface._();
}

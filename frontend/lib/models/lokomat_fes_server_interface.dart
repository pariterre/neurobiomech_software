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
  bool get isNidaqConnected => _isNidaqConnected;
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
        _socket!.listen(_listenToServerAnswer);

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
  Future<bool> send(Command command, [List<String>? parameters]) async {
    await _sendCommon(command, parameters);

    if (command == Command.shutdown || command == Command.quit) {
      _dispose();
      return false;
    }

    // Receive acknowledgment from the server
    String answer = await _waitForAnswer();

    if (command == Command.startNidaq) {
      answer = _finalizeStartNidaqCommand(answer);
    } else if (command == Command.stopNidaq) {
      answer = _finalizeStopNidaqCommand(answer);
    } else if (command == Command.startRecording) {
      answer = _finalizeStartCommand(answer);
    } else if (command == Command.stopRecording) {
      answer = _finalizeStopCommand(answer);
    } else if (command == Command.fetchData) {
      answer = _finalizeFetchCommand(answer);
    }

    return answer == "OK";
  }

  Future<void> _sendCommon(Command command, [List<String>? parameters]) async {
    if (!isInitialized) {
      log.severe('Communication not initialized');
      return;
    }

    // Construct and send the command
    _lastAnswer = null;

    String message = "${command.toInt()}:";
    if (parameters != null) {
      message += parameters.join(',');
    }
    _socket!.write(message);
    try {
      await _socket!.flush();
    } on SocketException {
      log.info('Connexion was closed by the server');
      return;
    }
    log.info('Sent command: $command');
  }

  Future<String> _waitForAnswer() async {
    while (_lastAnswer == null) {
      await Future.delayed(const Duration(milliseconds: 100));
    }
    return _lastAnswer!;
  }

  String _finalizeStartNidaqCommand(String answer) {
    _isNidaqConnected = true;
    return answer;
  }

  String _finalizeStopNidaqCommand(String answer) {
    _isNidaqConnected = false;
    _isRecording = false;
    return answer;
  }

  String _finalizeStartCommand(String answer) {
    _isRecording = true;
    _hasRecorded = true;
    return answer;
  }

  String _finalizeStopCommand(String answer) {
    _isRecording = false;
    return answer;
  }

  String _finalizeFetchCommand(String answer) {
    return "OK";
  }

  ///
  /// Close the connection to the server.
  void _dispose() {
    _socket?.close();
    _socket = null;

    _lastAnswer = null;
    _isNidaqConnected = false;
    _isRecording = false;
    _hasRecorded = false;

    log.info('Connection closed');
  }

  /// PRIVATE API ///
  Socket? _socket;
  String? _lastAnswer;

  bool _isNidaqConnected = false;
  bool _isRecording = false;
  bool _hasRecorded = false;

  final log = Logger('TcpCommunication');

  ///
  /// Listen to the server's acknowledgment.
  void _listenToServerAnswer(List<int> data) {
    _lastAnswer = utf8.decode(data);
  }

  // Prepare the singleton
  static final LokomatFesServerInterface _instance =
      LokomatFesServerInterface._();
  LokomatFesServerInterface._();
}

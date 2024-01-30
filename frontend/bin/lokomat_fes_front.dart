import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:logging/logging.dart';

enum Command {
  startRecording,
  stopRecording,
  stimulate,
  plotData,
  saveData,
  quit,
  shutdown;

  int toInt() {
    switch (this) {
      case Command.startRecording:
        return 1;
      case Command.stopRecording:
        return 2;
      case Command.stimulate:
        return 3;
      case Command.plotData:
        return 4;
      case Command.saveData:
        return 5;
      case Command.quit:
        return 6;
      case Command.shutdown:
        return 7;
      default:
        return 0;
    }
  }
}

class TcpCommunication {
  static Future<TcpCommunication> connect(
      {String serverIp = 'localhost', int serverPort = 4042}) async {
    while (true) {
      try {
        final socket = await Socket.connect(serverIp, serverPort);
        return TcpCommunication._(socket);
      } on SocketException {
        final log = Logger('TcpCommunication');
        log.info('Connection failed, retrying...');
        await Future.delayed(Duration(seconds: 1));
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
    // Construct and send the command
    _lastAcknowledge = null;

    String message = "${command.toInt()}:";
    if (parameters != null) {
      message += parameters.join(',');
    }
    _socket.write(message);
    try {
      await _socket.flush();
    } on SocketException {
      log.info('Connexion was closed by the server');
      return false;
    }
    log.info('Sent command: $command');

    if (command == Command.shutdown || command == Command.quit) {
      _dispose();
      return false;
    }

    // Receive acknowledgment from the server
    while (_lastAcknowledge == null) {
      await Future.delayed(Duration(milliseconds: 100));
    }
    return _lastAcknowledge == "OK";
  }

  void _dispose() {
    _socket.close();
    log.info('Connection closed');
  }

  /// INTERNAL ///
  final Socket _socket;
  String? _lastAcknowledge;
  final log = Logger('TcpCommunication');
  TcpCommunication._(this._socket) {
    _socket.listen((data) => _lastAcknowledge = utf8.decode(data));
  }
}

void main() async {
  // Configure logging
  Logger.root.onRecord.listen((record) {
    print('${record.level.name}: ${record.time}: ${record.message}');
  });

  final log = Logger('Main');

  // Create a Communication instance
  log.info('Connecting to server...');
  TcpCommunication communication = await TcpCommunication.connect();
  log.info('Connected to server');

  // Send "start_device" request
  await Future.delayed(Duration(seconds: 2));
  if (!(await communication.send(Command.startRecording))) return;
  await Future.delayed(Duration(seconds: 2));

  // Send "stimulate" request
  if (!(await communication.send(Command.stimulate, ['2.2']))) return;
  await Future.delayed(Duration(seconds: 2));

  // Send "stop_device" request
  if (!(await communication.send(Command.stopRecording))) return;
  await Future.delayed(Duration(seconds: 1));

  // Send "save_data" request
  if (!(await communication.send(Command.saveData, ["coucou.pkl"]))) return;

  // Send "shutdown" the server request
  await communication.send(Command.shutdown);

  return;
}

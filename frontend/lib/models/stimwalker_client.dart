import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:frontend/models/command.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/ack.dart';
import 'package:logging/logging.dart';

class StimwalkerClient {
  Socket? _socketCommand;
  Socket? _socketData;
  Socket? _socketLiveData;

  Command? _currentCommand;
  Completer<Ack>? _commandAckCompleter;
  Completer? _dataCompleter;

  bool _isConnectedToDelsysAnalog = false;
  bool _isConnectedToDelsysEmg = false;
  bool _isConnectedToLiveData = false;
  Data liveData = Data(
      t0: DateTime.now().millisecondsSinceEpoch / 1000,
      analogChannelCount: 144,
      emgChannelCount: 16);

  bool _isRecording = false;
  bool _hasRecorded = false;

  final _log = Logger('TcpCommunication');

  /// PUBLIC API ///

  ///
  /// Get the singleton instance of the TcpCommunication class.
  static StimwalkerClient get instance => _instance;

  ///
  /// If the communication initialized.
  bool get isInitialized =>
      _socketCommand != null && _socketData != null && _socketLiveData != null;
  bool get isConnectedToDelsysAnalog => _isConnectedToDelsysAnalog;
  bool get isConnectedToDelsysEmg => _isConnectedToDelsysEmg;
  bool get isConnectedToLiveData => _isConnectedToLiveData;

  bool get isRecording => _isRecording;
  bool get hasRecorded => _hasRecorded;

  ///
  /// Initialize the communication with the server. If the connection fails,
  /// the function will retry until it succeeds or the number of retries is
  /// reached.
  Future<void> initialize({
    String serverIp = 'localhost',
    int commandPort = 5000,
    int dataPort = 5001,
    int liveDataPort = 5002,
    int? nbOfRetries,
  }) async {
    if (isInitialized) return;

    _log.info('Initializing communication with the server');
    await _connectSockets(
        serverIp: serverIp,
        commandPort: commandPort,
        dataPort: dataPort,
        liveDataPort: liveDataPort,
        nbOfRetries: nbOfRetries);

    if (!(await _send(Command.handshake, null))) {
      _log.severe('Handshake failed');
      await disconnect();
      return;
    }

    _log.info('Communication initialized');
  }

  ///
  /// Close the connection to the server.
  Future<void> disconnect() async {
    if (_commandAckCompleter != null && !_commandAckCompleter!.isCompleted) {
      _commandAckCompleter?.complete(Ack.nok);
    }
    if (_dataCompleter != null && !_dataCompleter!.isCompleted) {
      _dataCompleter?.complete();
    }

    _disconnectSockets();

    _isConnectedToDelsysAnalog = false;
    _isConnectedToDelsysEmg = false;
    _isConnectedToLiveData = false;
    _isRecording = false;
    _hasRecorded = false;

    _log.info('Connection closed');
  }

  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int dataPort,
    required int liveDataPort,
    required int? nbOfRetries,
  }) async {
    _socketCommand = await _connectToSocket(
        ipAddress: serverIp,
        port: commandPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveCommandAck);
    _socketData = await _connectToSocket(
        ipAddress: serverIp,
        port: dataPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveData);
    _socketLiveData = await _connectToSocket(
        ipAddress: serverIp,
        port: liveDataPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveLiveData);
  }

  Future<void> _disconnectSockets() async {
    await _socketCommand?.close();
    _socketCommand = null;

    await _socketData?.close();
    _socketData = null;

    await _socketLiveData?.close();
    _socketLiveData = null;
  }

  ///
  /// Send a command to the server. If the command requires parameters, they
  /// can be passed as a list of strings.
  /// Returns true if the server returned OK and if the connexion to
  /// the server is still alive, false otherwise.
  Future<bool> send(Command command, {List<String>? parameters}) async {
    if (!command.isImplemented) {
      _log.severe('Command $command is not implemented');
      return false;
    }

    // Check if the command is reserved to internal
    if (command.isReserved) {
      _log.severe('Command $command is reserved and should not be called');
      return false;
    }

    return _send(command, parameters);
  }

  Future<bool> _send(Command command, List<String>? parameters) async {
    if (!isInitialized) {
      _log.severe('Communication not initialized');
      return false;
    }

    // Format the command and send the messsage to the server
    if (await _performSend(command) == Ack.ok) {
      _log.info('Sent command: $command');
      return true;
    } else {
      _log.severe('Command $command failed');
      return false;
    }
  }

  Future<Ack> _performSend(Command command) async {
    // Construct and send the command
    try {
      _currentCommand = command;
      _commandAckCompleter = Completer<Ack>();
      _socketCommand!.add(command.toPacket());
      await _socketCommand!.flush();
      return await _commandAckCompleter!.future;
    } on SocketException {
      _log.info('Connexion was closed by the server');
      disconnect();
      return Ack.nok;
    }
  }

  ///
  /// Get a response from the server to a command
  void _receiveCommandAck(List<int> response) {
    if (_commandAckCompleter == null || _currentCommand == null) {
      _log.severe('Got a response without a command');
      return;
    }
    if (_commandAckCompleter!.isCompleted) {
      _log.severe('Got a response but the command was already responded');
      return;
    }

    final ack = Ack.parse(response);
    if (ack == Ack.ok) {
      _setFlagsFromCommand(_currentCommand!);
    }
    _currentCommand = null;
    _commandAckCompleter!.complete(ack);
  }

  void _setFlagsFromCommand(Command command) {
    switch (command) {
      case Command.connectDelsysAnalog:
      case Command.disconnectDelsysAnalog:
        _isConnectedToDelsysAnalog = command == Command.connectDelsysAnalog;
        _isRecording = false;
        resetLiveData();
        break;

      case Command.disconnectDelsysEmg:
      case Command.connectDelsysEmg:
        _isConnectedToDelsysEmg = command == Command.connectDelsysEmg;
        _isRecording = false;
        resetLiveData();
        break;

      case Command.startRecording:
      case Command.stopRecording:
        _isRecording = command == Command.startRecording;
        _hasRecorded = true;
        break;

      case Command.handshake:
      case Command.connectMagstim:
      case Command.disconnectMagstim:
      case Command.getLastTrial:
        break;
    }
  }

  void resetLiveData() => liveData = Data(
      t0: DateTime.now().millisecondsSinceEpoch / 1000,
      analogChannelCount: liveData.delsysAnalog.channelCount,
      emgChannelCount: liveData.delsysEmg.channelCount);

  void _receiveData(List<int> data) {
    _log.info('Data received: ${utf8.decode(data)}');
  }

  void _receiveLiveData(List<int> data) {
    _log.info('Live data received: ${utf8.decode(data)}');
  }

  // Prepare the singleton
  static final StimwalkerClient _instance = StimwalkerClient._();
  StimwalkerClient._();

  Future<Socket?> _connectToSocket({
    required String ipAddress,
    required int port,
    required int? nbOfRetries,
    required Function(List<int>) hasDataCallback,
  }) async {
    while (nbOfRetries == null || nbOfRetries > 0) {
      try {
        final socket = await Socket.connect(ipAddress, port);
        socket.listen(hasDataCallback);

        return socket;
      } on SocketException {
        final log = Logger('TcpCommunication');
        log.info('Connection failed, retrying...');
        await Future.delayed(const Duration(seconds: 1));
        nbOfRetries = nbOfRetries == null ? null : nbOfRetries - 1;
      }
    }
    return null;
  }
}

class StimwalkerClientMock extends StimwalkerClient {
  StimwalkerClientMock._() : super._();

  static StimwalkerClient get instance => _instance;
  static final StimwalkerClient _instance = StimwalkerClientMock._();

  bool _isMockInitialized = false;

  @override
  bool get isInitialized => _isMockInitialized;

  @override
  Future<void> initialize({
    String serverIp = 'localhost',
    int commandPort = 5000,
    int dataPort = 5001,
    int liveDataPort = 5002,
    int? nbOfRetries,
  }) async {
    if (isInitialized) return;
    _isMockInitialized = true;
  }

  @override
  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int dataPort,
    required int liveDataPort,
    required int? nbOfRetries,
  }) async {}

  @override
  Future<void> _disconnectSockets() async {}
}

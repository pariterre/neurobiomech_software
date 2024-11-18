import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:frontend/models/command.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/ack.dart';
import 'package:logging/logging.dart';

const _protocolVersion = 1;
const _serverHeaderLength = 16;

class StimwalkerClient {
  Socket? _socketCommand;
  Socket? _socketResponse;
  Socket? _socketLiveData;

  Command? _currentCommand;
  Completer<Ack>? _commandAckCompleter;
  Completer? _responseCompleter;
  Future? get onDataArrived => _responseCompleter?.future;

  int? _expectedResponseLength;
  final _responseGetLastTrial = <int>[];

  int? _expectedLiveDataLength;
  DateTime? _lastLiveDataTimestamp;
  final _responseLiveData = <int>[];
  var _liveDataCompleter = Completer();

  Function()? _onNewLiveData;

  bool _isConnectedToDelsysAnalog = false;
  bool _isConnectedToDelsysEmg = false;
  bool _isConnectedToLiveData = false;
  Data liveData = Data(
      initialTime: DateTime.now(),
      analogChannelCount: 3 * 16,
      emgChannelCount: 16);
  Data lastTrialData = Data(
      initialTime: DateTime.now(),
      analogChannelCount: 3 * 16,
      emgChannelCount: 16);
  Duration liveDataTimeWindow = const Duration(seconds: 3);

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
      _socketCommand != null &&
      _socketResponse != null &&
      _socketLiveData != null;
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
    int responsePort = 5001,
    int liveDataPort = 5002,
    int? nbOfRetries,
    required Function() onConnexionLost,
    required Function() onNewLiveData,
  }) async {
    if (isInitialized) return;

    _log.info('Initializing communication with the server');
    await _connectSockets(
        serverIp: serverIp,
        commandPort: commandPort,
        responsePort: responsePort,
        liveDataPort: liveDataPort,
        nbOfRetries: nbOfRetries,
        onConnexionLost: onConnexionLost);

    if (!(await _send(Command.handshake, null))) {
      _log.severe('Handshake failed');
      await disconnect();
      return;
    }

    _lastLiveDataTimestamp = null;
    _expectedLiveDataLength = null;
    _onNewLiveData = onNewLiveData;

    _log.info('Communication initialized');
  }

  ///
  /// Close the connection to the server.
  Future<void> disconnect() async {
    if (_commandAckCompleter != null && !_commandAckCompleter!.isCompleted) {
      _commandAckCompleter?.complete(Ack.nok);
    }
    if (_responseCompleter != null && !_responseCompleter!.isCompleted) {
      _responseCompleter?.complete();
    }

    _disconnectSockets();

    _isConnectedToDelsysAnalog = false;
    _isConnectedToDelsysEmg = false;
    _isConnectedToLiveData = false;
    _isRecording = false;
    _hasRecorded = false;

    _commandAckCompleter = null;
    _responseCompleter = null;
    _expectedResponseLength = null;
    _responseGetLastTrial.clear();
    _lastLiveDataTimestamp = null;
    _expectedLiveDataLength = null;
    _responseLiveData.clear();
    _liveDataCompleter = Completer();
    _onNewLiveData = null;

    _log.info('Connection closed');
  }

  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int responsePort,
    required int liveDataPort,
    required int? nbOfRetries,
    required Function()? onConnexionLost,
  }) async {
    _socketCommand = await _connectToSocket(
        ipAddress: serverIp,
        port: commandPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveCommandAck,
        onConnexionLost: onConnexionLost);
    _socketResponse = await _connectToSocket(
        ipAddress: serverIp,
        port: responsePort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveResponse,
        onConnexionLost: onConnexionLost);
    _socketLiveData = await _connectToSocket(
        ipAddress: serverIp,
        port: liveDataPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveLiveData,
        onConnexionLost: onConnexionLost);
  }

  Future<void> _disconnectSockets() async {
    await _socketCommand?.close();
    _socketCommand = null;

    await _socketResponse?.close();
    _socketResponse = null;

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
    if (command == Command.getLastTrial) {
      _prepareLastTrialResponse();
    }

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

    final version = _parseVersionFromPacket(response);
    if (version != _protocolVersion) {
      _log.severe(
          'Protocol version mismatch, expected $_protocolVersion, got $version. '
          'Please update the client.');
      disconnect();
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

  void resetLiveData() => liveData.clear(initialTime: DateTime.now());

  void _prepareLastTrialResponse() {
    lastTrialData.clear();
    _responseCompleter = Completer();
    _expectedResponseLength = null;
    _responseGetLastTrial.clear();
  }

  void _receiveResponse(List<int> response) {
    if (!isInitialized) return;

    if (_currentCommand == Command.getLastTrial &&
        _expectedResponseLength == null) {
      lastTrialData.clear(initialTime: _parseTimestampFromPacket(response));
      _expectedResponseLength = _parseDataLengthFromPacket(response);
      if (response.length > _serverHeaderLength) {
        // If more data came at once, recursively call the function with the rest
        // of the data.
        _receiveResponse(response.sublist(_serverHeaderLength));
      }
      return;
    }

    _responseGetLastTrial.addAll(response);
    if (_responseGetLastTrial.length < _expectedResponseLength!) {
      _log.info('Received ${_responseGetLastTrial.length} bytes, waiting for '
          '$_expectedResponseLength');
      return;
    } else if (_responseGetLastTrial.length > _expectedResponseLength!) {
      _log.severe('Received more data than expected, dropping everything');
      _responseGetLastTrial.clear();
      _expectedResponseLength = null;
      return;
    }

    // Convert the data to a string (from json)
    _expectedResponseLength = null;
    final jsonRaw = json.decode(utf8.decode(_responseGetLastTrial));
    if (jsonRaw != null) {
      lastTrialData.appendFromJson(jsonRaw as List);
    }
    _responseCompleter!.complete();
  }

  Future<void> _receiveLiveData(List<int> response) async {
    if (response.isEmpty || !isInitialized) return;

    if (_expectedLiveDataLength == null) {
      _liveDataCompleter = Completer();
      _lastLiveDataTimestamp = _parseTimestampFromPacket(response);
      _expectedLiveDataLength = _parseDataLengthFromPacket(response);
      if (response.length > _serverHeaderLength) {
        // If more data came at once, recursively call the function with the rest
        // of the data.
        _receiveLiveData(response.sublist(_serverHeaderLength));
      }
      return;
    }

    _responseLiveData.addAll(response);
    if (_responseLiveData.length < _expectedLiveDataLength!) {
      // Waiting for the rest of the live data
      return;
    } else if (_responseLiveData.length > _expectedLiveDataLength!) {
      // We received the data for the next time frame, parse the current, then
      // call the function again with the rest of the data.
      final responseRemaining =
          _responseLiveData.sublist(_expectedLiveDataLength!);
      _responseLiveData.removeRange(
          _expectedLiveDataLength!, _responseLiveData.length);
      _liveDataCompleter.future
          .then((_) => _receiveLiveData(responseRemaining));
    }

    // Convert the data to a string (from json)
    try {
      final dataList = json.decode(utf8.decode(_responseLiveData)) as List;
      if (liveData.isEmpty) {
        // If the live data were reset, we need to clear again with the current time stamp
        liveData.clear(initialTime: _lastLiveDataTimestamp);
      }
      liveData.appendFromJson(dataList);
      liveData.dropBefore(_lastLiveDataTimestamp!.subtract(liveDataTimeWindow));
    } catch (e) {
      _log.severe('Error while parsing live data: $e, resetting');
      resetLiveData();
    }
    _responseLiveData.clear();
    _expectedLiveDataLength = null;
    _liveDataCompleter.complete();

    if (_onNewLiveData != null) _onNewLiveData!();
  }

  int _parseVersionFromPacket(List<int> data) {
    // Bit shifting from little-ending to 32-bits integer, starting from the 1st byte
    return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
  }

  DateTime _parseTimestampFromPacket(List<int> data) {
    // Bit shifting from little-endian to 64-bits integer, starting from the 5th byte
    return DateTime.fromMillisecondsSinceEpoch(data[4] +
        (data[5] << 8) +
        (data[6] << 16) +
        (data[7] << 24) +
        (data[8] << 32) +
        (data[9] << 40) +
        (data[10] << 48) +
        (data[11] << 56));
  }

  int _parseDataLengthFromPacket(List<int> data) {
    // Bit shifting from little-ending to 32-bits integer, starting from the 13th byte
    return data[12] + (data[13] << 8) + (data[14] << 16) + (data[15] << 24);
  }

  // Prepare the singleton
  static final StimwalkerClient _instance = StimwalkerClient._();
  StimwalkerClient._();

  Future<Socket?> _connectToSocket({
    required String ipAddress,
    required int port,
    required int? nbOfRetries,
    required Function(List<int>) hasDataCallback,
    required Function()? onConnexionLost,
  }) async {
    while (nbOfRetries == null || nbOfRetries > 0) {
      try {
        final socket = await Socket.connect(ipAddress, port);
        socket.listen(hasDataCallback, onDone: () {
          if (onConnexionLost == null) return;
          _log.info('Connection closed');
          disconnect();
          onConnexionLost();
        });

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
    int responsePort = 5001,
    int liveDataPort = 5002,
    int? nbOfRetries,
    required Function() onConnexionLost,
    required Function() onNewLiveData,
  }) async {
    if (isInitialized) return;
    _isMockInitialized = true;

    _onNewLiveData = onNewLiveData;
  }

  @override
  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int responsePort,
    required int liveDataPort,
    required int? nbOfRetries,
    required Function()? onConnexionLost,
  }) async {}

  @override
  Future<void> _disconnectSockets() async {}
}

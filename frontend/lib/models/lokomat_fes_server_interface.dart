import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:frontend/models/commands.dart';
import 'package:frontend/models/data.dart';
import 'package:logging/logging.dart';

List<Command> _commandsThatRequireDataResponse = [
  Command.startNidaq,
  Command.fetchData,
];

class LokomatFesServerInterface {
  Socket? _socketCommand;
  Socket? _socketData;
  String? _commandAnswer;
  String? _dataAnswer;

  bool _isNidaqConnected = false;
  bool _isRecording = false;
  bool _hasRecorded = false;

  bool _isContinousDataActive = false;
  bool get isContinousDataActive => _isContinousDataActive;
  Data? _continousData;
  Data get continousData => _continousData!;

  bool _isSendingCommand = false;
  bool _isReceingData = false;

  final _log = Logger('TcpCommunication');

  /// PUBLIC API ///

  ///
  /// Get the singleton instance of the TcpCommunication class.
  static LokomatFesServerInterface get instance => _instance;

  ///
  /// If the communication initialized.
  bool get isInitialized => _socketCommand != null;
  bool get isNidaqConnected => _isNidaqConnected;
  bool get isRecording => _isRecording;
  bool get hasRecorded => _hasRecorded;

  ///
  /// Initialize the communication with the server. If the connection fails,
  /// the function will retry until it succeeds or the number of retries is
  /// reached.
  Future<void> initialize({
    String serverIp = 'localhost',
    int commandPort = 4042,
    int dataPort = 4043,
    int? nbOfRetries,
  }) async {
    if (isInitialized) return;

    _socketCommand = await _connectToServer(
        ipAddress: serverIp,
        port: commandPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _listenToCommunicationAnswer);
    _socketData = await _connectToServer(
        ipAddress: serverIp,
        port: dataPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _listenToDataAnswer);
  }

  ///
  /// Send a command to the server. If the command requires parameters, they
  /// can be passed as a list of strings.
  /// Returns true if the command was successfully sent the connexion to
  /// the server is still alive, false otherwise.
  /// If the command was "shutdown" or "quit", the connection is closed. By its
  /// nature, the "shutdown" or "quit" commands will always return false.
  Future<bool> send(Command command, {List<String>? parameters}) async {
    // Check if the command is reserved
    if (command.isReserved) {
      _log.severe('Command $command is reserved and should not be called');
      return false;
    }
    return _send(command, parameters);
  }

  Future<bool> _send(Command command, List<String>? parameters) async {
    await _preSendCommand(command, parameters);

    // Send the command to the server
    await _sendCommand(command, parameters);

    // Manage the response from the server
    return await _postSendCommand(command);
  }

  Future<void> _preSendCommand(
      Command command, List<String>? parameters) async {
    // Wait for the server to be ready to receive data in case data are expected
    if (_commandsThatRequireDataResponse.contains(command)) {
      while (_isReceingData) {
        await Future.delayed(const Duration(milliseconds: 50));
      }
      _isReceingData = true;
    }

    // Wait for the server to be ready to receive a new command
    while (_isSendingCommand) {
      await Future.delayed(const Duration(milliseconds: 50));
    }
    _isSendingCommand = true;
  }

  Future<void> _sendCommand(Command command, [List<String>? parameters]) async {
    if (!isInitialized) {
      _log.severe('Communication not initialized');
      return;
    }

    // Construct and send the command
    _commandAnswer = null;

    String message = "${command.toInt()}:";
    if (parameters != null) {
      message += parameters.join(',');
    }
    _socketCommand!.write(message);
    try {
      await _socketCommand!.flush();
    } on SocketException {
      _log.info('Connexion was closed by the server');
      return;
    }
    _log.info('Sent command: $command');
  }

  Future<bool> _postSendCommand(Command command) async {
    switch (command) {
      case Command.quit:
      case Command.shutdown:
        _dispose();
        // Quick exit as the connection now closed
        return false;

      case Command.startNidaq:
        _isNidaqConnected = true;
        _prepareAutomaticDataFetch();
        break;

      case Command.stopNidaq:
        _isNidaqConnected = false;
        _isRecording = false;
        break;

      case Command.startRecording:
        _isRecording = true;
        _hasRecorded = true;
        break;

      case Command.stopRecording:
        _isRecording = false;
        break;

      case Command.fetchData:
        _manageFetchedData();
        break;

      case Command.stimulate:
      case Command.plotData:
      case Command.saveData:
        break;
    }

    // Receive acknowledgment from the server
    final answer = await _waitForCommandAnswer();
    _isSendingCommand = false;
    return answer == "OK";
  }

  Future<String> _waitForCommandAnswer() async {
    while (_commandAnswer == null) {
      await Future.delayed(const Duration(milliseconds: 100));
    }
    final message = _commandAnswer!;
    _commandAnswer = null;
    return message;
  }

  Future<String> _waitForDataAnswer() async {
    while (_dataAnswer == null) {
      await Future.delayed(const Duration(milliseconds: 100));
    }
    final message = _dataAnswer!;
    _dataAnswer = null;
    _isReceingData = false;
    return message;
  }

  Future<void> _prepareAutomaticDataFetch() async {
    final answer = jsonDecode(await _waitForDataAnswer());

    _continousData = Data(
        t0: answer['t0'],
        nidaqT0Offset: answer["nidaqT0Offset"],
        nbNidaqChannels: answer["nidaqNbChannels"],
        nbRehastimChannels: answer["rehastimNbChannels"]);
  }

  void startAutomaticDataFetch() {
    if (_continousData == null) {
      _log.severe('Data not initialized');
      return;
    }

    _isContinousDataActive = true;
    Timer.periodic(const Duration(milliseconds: 1000), (timer) async {
      if (_isNidaqConnected && _isContinousDataActive) {
        await _send(Command.fetchData, []);
      } else {
        timer.cancel();
      }
    });
  }

  void stopAutomaticDataFetch() {
    _isContinousDataActive = false;
  }

  Future<void> _manageFetchedData() async {
    /// This is for online data fetching. We do not mind missing data so just
    /// disregard the error if the data is not received properly
    try {
      final dataRaw = await _waitForDataAnswer();
      _isSendingCommand = false;
      _isReceingData = false;
      final data = jsonDecode(dataRaw);
      _continousData!.appendFromJson(data);
    } catch (e) {
      _log.severe('Error while decoding data: $e');
    }
    _log.info('Revieved data');
  }

  ///
  /// Close the connection to the server.
  void _dispose() {
    _socketCommand?.close();
    _socketCommand = null;

    _socketData?.close();
    _socketData = null;

    _isSendingCommand = false;
    _isReceingData = false;

    _continousData = null;
    _isContinousDataActive = false;

    _commandAnswer = null;
    _isNidaqConnected = false;
    _isRecording = false;
    _hasRecorded = false;

    _log.info('Connection closed');
  }

  /// PRIVATE API ///

  ///
  /// Listen to the server's acknowledgment.
  void _listenToCommunicationAnswer(List<int> data) {
    _commandAnswer = utf8.decode(data);
    _log.info('Received command answer: $_commandAnswer');
  }

  void _listenToDataAnswer(List<int> data) {
    final message = utf8.decode(data);
    _dataAnswer = message;
  }

  // Prepare the singleton
  static final LokomatFesServerInterface _instance =
      LokomatFesServerInterface._();
  LokomatFesServerInterface._();
}

Future<Socket?> _connectToServer({
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

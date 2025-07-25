import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math';
import 'dart:typed_data';

import 'package:frontend/managers/predictions_manager.dart';
import 'package:frontend/models/ack.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/prediction_model.dart';
import 'package:frontend/utils/generic_listener.dart';
import 'package:logging/logging.dart';

const _serverHeaderLength = 16;

class NeurobioClient {
  static const communicationProtocolVersion = 2;

  Socket? _socketCommand;
  Socket? _socketResponse;
  Socket? _socketLiveAnalogsData;
  Socket? _socketLiveAnalyses;

  Command? _currentCommand;
  Completer<Ack>? _commandAckCompleter;
  Completer? _responseCompleter;
  Future? get onResponseArrived => _responseCompleter?.future;

  int? _expectedResponseLength;
  final _responseData = <int>[];

  int? _expectedLiveAnalogsDataLength;
  DateTime? _lastLiveAnalogsDataTimestamp;
  final _rawLiveAnalogsList = <int>[];
  var _liveAnalogsDataCompleter = Completer();
  Function()? _onNewLiveAnalogsData;

  int? _expectedLiveAnalysesLength;
  DateTime? _lastLiveAnalysesTimestamp;
  final _rawLiveAnalysesList = <int>[];
  var _liveAnalysesDataCompleter = Completer();
  Function()? _onNewLiveAnalyses;

  bool _isConnectedToDelsysAnalog = false;
  bool _isConnectedToDelsysEmg = false;
  bool _isConnectedToLiveAnalogsData = false;
  bool _isConnectedToLiveAnalyses = false;
  Data liveAnalogsData = Data(
      dataGenericType: DataGenericTypes.analogs,
      initialTime: DateTime.now(),
      analogChannelCount: 9 * 16,
      emgChannelCount: 16,
      isFromLiveData: true);
  Data lastTrialAnalogsData = Data(
      dataGenericType: DataGenericTypes.analogs,
      initialTime: DateTime.now(),
      analogChannelCount: 9 * 16,
      emgChannelCount: 16,
      isFromLiveData: false);
  Duration liveAnalogsDataTimeWindow = const Duration(seconds: 3);
  Data liveAnalyses = Data(
      dataGenericType: DataGenericTypes.predictions,
      initialTime: DateTime.now(),
      analogChannelCount: 0,
      emgChannelCount: 0,
      isFromLiveData: true);
  Duration liveAnalysesTimeWindow = const Duration(seconds: 3);

  final onMessageFromBackend = GenericListener<Function(Ack)>();

  bool _isRecording = false;

  final _log = Logger('TcpCommunication');

  /// PUBLIC API ///

  ///
  /// Get the singleton instance of the TcpCommunication class.
  static NeurobioClient get instance => _instance;

  ///
  /// If the communication initialized.
  bool get isInitialized =>
      _socketCommand != null &&
      _socketResponse != null &&
      _socketLiveAnalogsData != null &&
      _socketLiveAnalyses != null;
  bool get isConnectedToDelsysAnalog => _isConnectedToDelsysAnalog;
  bool get isConnectedToDelsysEmg => _isConnectedToDelsysEmg;
  bool get isConnectedToLiveAnalogsData => _isConnectedToLiveAnalogsData;
  bool get isConnectedToLiveAnalyses => _isConnectedToLiveAnalyses;

  bool get isRecording => _isRecording;
  bool get hasRecorded => !isRecording && lastTrialAnalogsData.isNotEmpty;

  ///
  /// Initialize the communication with the server. If the connection fails,
  /// the function will retry until it succeeds or the number of retries is
  /// reached.
  Future<void> initialize({
    String serverIp = 'localhost',
    int commandPort = 5000,
    int responsePort = 5001,
    int liveAnalogsDataPort = 5002,
    int liveAnalysesPort = 5003,
    int? nbOfRetries,
    required Function() onConnexionLost,
    required Function() onNewLiveAnalogsData,
    required Function() onNewLiveAnalyses,
  }) async {
    if (isInitialized) return;

    _log.info('Initializing communication with the server');
    await _connectSockets(
        serverIp: serverIp,
        commandPort: commandPort,
        responsePort: responsePort,
        liveAnalogsDataPort: liveAnalogsDataPort,
        liveAnalysesPort: liveAnalysesPort,
        nbOfRetries: nbOfRetries,
        onConnexionLost: onConnexionLost);

    if (!(await _send(Command.handshake, null))) {
      _log.severe('Handshake failed');
      await disconnect();
      return;
    }

    _lastLiveAnalogsDataTimestamp = null;
    _expectedLiveAnalogsDataLength = null;
    _onNewLiveAnalogsData = onNewLiveAnalogsData;
    _onNewLiveAnalyses = onNewLiveAnalyses;

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
    _isConnectedToLiveAnalogsData = false;
    _isConnectedToLiveAnalyses = false;
    _isRecording = false;
    liveAnalogsData.clear(initialTime: DateTime.now());
    liveAnalyses.clear(initialTime: DateTime.now(), fullReset: true);
    lastTrialAnalogsData.clear(initialTime: DateTime.now());

    _commandAckCompleter = null;
    _responseCompleter = null;
    _expectedResponseLength = null;
    _responseData.clear();

    _lastLiveAnalogsDataTimestamp = null;
    _rawLiveAnalogsList.clear();
    _expectedLiveAnalogsDataLength = null;
    _liveAnalogsDataCompleter = Completer();
    _onNewLiveAnalogsData = null;

    _lastLiveAnalysesTimestamp = null;
    _rawLiveAnalysesList.clear();
    _expectedLiveAnalysesLength = null;
    _liveAnalysesDataCompleter = Completer();
    _onNewLiveAnalyses = null;

    _log.info('Connection closed');
  }

  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int responsePort,
    required int liveAnalogsDataPort,
    required int liveAnalysesPort,
    required int? nbOfRetries,
    required Function()? onConnexionLost,
  }) async {
    final id = Random().nextInt(0xEFFFFFFE) + 0x10000000;

    _socketCommand = await _connectToSocket(
        id: id,
        ipAddress: serverIp,
        port: commandPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveCommandAck,
        onConnexionLost: onConnexionLost);
    _socketResponse = await _connectToSocket(
        id: id,
        ipAddress: serverIp,
        port: responsePort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveResponse,
        onConnexionLost: onConnexionLost);
    _socketLiveAnalogsData = await _connectToSocket(
        id: id,
        ipAddress: serverIp,
        port: liveAnalogsDataPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveLiveAnalogsData,
        onConnexionLost: onConnexionLost);
    _isConnectedToLiveAnalogsData = true;
    _socketLiveAnalyses = await _connectToSocket(
        id: id,
        ipAddress: serverIp,
        port: liveAnalysesPort,
        nbOfRetries: nbOfRetries,
        hasDataCallback: _receiveLiveAnalyses,
        onConnexionLost: onConnexionLost);
    _isConnectedToLiveAnalyses = true;
  }

  Future<void> _disconnectSockets() async {
    await _socketCommand?.close();
    _socketCommand = null;

    await _socketResponse?.close();
    _socketResponse = null;

    await _socketLiveAnalogsData?.close();
    _socketLiveAnalogsData = null;

    await _socketLiveAnalyses?.close();
    _socketLiveAnalyses = null;
  }

  ///
  /// Send a command to the server. If the command requires parameters, they
  /// can be passed as a list of strings.
  /// Returns true if the server returned OK and if the connexion to
  /// the server is still alive, false otherwise.
  Future<bool> send(Command command, {Map<String, dynamic>? parameters}) async {
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

  Future<bool> _send(Command command, Map<String, dynamic>? parameters) async {
    if (!isInitialized) {
      _log.severe('Communication not initialized');
      return false;
    }

    // Format the command and send the messsage to the server
    if (await _performSend(command) == Ack.ok) {
      _log.info('Sent command: $command');

      if (parameters != null) {
        // Reset the current command so we can receive the Ack again
        _currentCommand = command;
        await _performSendParameters(parameters);
      }

      return true;
    } else {
      _log.severe('Command $command failed');
      return false;
    }
  }

  Future<Ack> _performSend(Command command) async {
    switch (command) {
      case Command.handshake:
      case Command.connectDelsysAnalog:
      case Command.connectDelsysEmg:
      case Command.connectMagstim:
      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.disconnectMagstim:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        break;
      case Command.getStates:
        _prepareResponseCompleter();
      case Command.getLastTrial:
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

  Future<Ack> _performSendParameters(Map<String, dynamic> parameters) async {
    // Construct and send the parameters
    try {
      _commandAckCompleter = Completer<Ack>();
      final packets = utf8.encode(json.encode(parameters));
      _socketResponse!.add(Command.constructPacket(command: packets.length));
      _socketResponse!.add(packets);
      await _socketResponse!.flush();
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
    if (version != communicationProtocolVersion) {
      _log.severe(
          'Protocol version mismatch, expected $communicationProtocolVersion, got $version. '
          'Please update the client.');
      disconnect();
      return;
    }

    final ack = Ack.parse(response);
    if (ack == Ack.ok) {
      _setFlagsFromCommand(_currentCommand!);
    }
    if (!_currentCommand!.hasDataResponse) _currentCommand = null;
    _commandAckCompleter!.complete(ack);
  }

  void _setFlagsFromCommand(Command command) {
    switch (command) {
      case Command.connectDelsysAnalog:
      case Command.disconnectDelsysAnalog:
        _isConnectedToDelsysAnalog = command == Command.connectDelsysAnalog;
        _isRecording = false;
        resetLiveAnalogsData();
        break;

      case Command.disconnectDelsysEmg:
      case Command.connectDelsysEmg:
        _isConnectedToDelsysEmg = command == Command.connectDelsysEmg;
        _isRecording = false;
        resetLiveAnalogsData();
        break;

      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
        break;

      case Command.startRecording:
      case Command.stopRecording:
        _isRecording = command == Command.startRecording;
        break;

      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        break;

      case Command.handshake:
      case Command.getStates:
      case Command.connectMagstim:
      case Command.disconnectMagstim:
      case Command.getLastTrial:
        break;
    }
  }

  void _setFlagsFromStates(Map<String, dynamic> states) {
    _isConnectedToDelsysEmg = false;
    _isConnectedToDelsysAnalog = false;
    _isRecording = false;
    if (states.containsKey('DelsysEmgDevice')) {
      _isConnectedToDelsysEmg =
          states['DelsysEmgDevice']['is_connected'] ?? false;
      _isRecording =
          _isRecording || (states['DelsysEmgDevice']['is_recording'] ?? false);
    }
    if (states.containsKey('DelsysAnalogDevice')) {
      _isConnectedToDelsysAnalog =
          states['DelsysAnalogDevice']['is_connected'] ?? false;
      _isRecording = _isRecording ||
          (states['DelsysAnalogDevice']['is_recording'] ?? false);
    }

    if (_isRecording) resetLiveAnalogsData();
  }

  void _prepareLastTrialResponse() {
    lastTrialAnalogsData.clear();
    _prepareResponseCompleter();
  }

  void _prepareResponseCompleter() {
    _responseCompleter = Completer();
    _expectedResponseLength = null;
    _responseData.clear();
  }

  void resetLiveAnalyses() => liveAnalyses.clear(initialTime: DateTime.now());

  void resetLiveAnalogsData() =>
      liveAnalogsData.clear(initialTime: DateTime.now());

  void _receiveResponse(List<int> response) {
    if (!isInitialized) return;

    if (_expectedResponseLength == null) {
      switch (_currentCommand) {
        case Command.handshake:
        case Command.connectDelsysAnalog:
        case Command.connectDelsysEmg:
        case Command.connectMagstim:
        case Command.zeroDelsysAnalog:
        case Command.zeroDelsysEmg:
        case Command.disconnectDelsysAnalog:
        case Command.disconnectDelsysEmg:
        case Command.disconnectMagstim:
        case Command.startRecording:
        case Command.stopRecording:
        case Command.addAnalyzer:
        case Command.removeAnalyzer:
          _log.severe('Received data for an unknown command');
          throw StateError('Received data for an unknown command');
        case Command.getStates:
          break;
        case Command.getLastTrial:
          lastTrialAnalogsData.clear(
              initialTime: _parseTimestampFromPacket(response));
          return;
        case null:
          final ack = Ack.parse(response);
          switch (ack) {
            case Ack.ok:
            case Ack.nok:
              _log.severe('Response received without a command');
              throw StateError('Response received without a command');
            case Ack.statesChanged:
              send(Command.getStates);
              onMessageFromBackend.notifyListeners((callback) => callback(ack));
              break;
          }
      }
      _expectedResponseLength = _parseDataFromResponsePacket(response);
      if (response.length > _serverHeaderLength) {
        // If more data came at once, recursively call the function with the rest
        // of the data.
        _receiveResponse(response.sublist(_serverHeaderLength));
      }
      return;
    }

    _responseData.addAll(response);
    _log.info(
        'Received ${_responseData.length} / $_expectedResponseLength bytes');
    if (_responseData.length < _expectedResponseLength!) {
      // Waiting for the rest of the response
      return;
    } else if (_responseData.length > _expectedResponseLength!) {
      _log.severe('Received more data than expected, dropping everything');
      _responseData.clear();
      _expectedResponseLength = null;
      return;
    }

    // Convert the data to a string (from json)
    _expectedResponseLength = null;
    switch (_currentCommand) {
      case Command.handshake:
      case Command.connectDelsysAnalog:
      case Command.connectDelsysEmg:
      case Command.connectMagstim:
      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.disconnectMagstim:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
      case null:
        break;
      case Command.getStates:
        // To string
        final jsonRaw =
            json.decode(utf8.decode(_responseData)) as Map<String, dynamic>?;
        if (jsonRaw != null) {
          if (jsonRaw.containsKey('connected_devices') &&
              jsonRaw['connected_devices'] != null) {
            _setFlagsFromStates(jsonRaw['connected_devices']);
            resetLiveAnalogsData();
          }
          if (jsonRaw.containsKey('connected_analyzers') &&
              jsonRaw['connected_analyzers'] != null) {
            _isConnectedToLiveAnalyses = true;
            final manager = PredictionsManager.instance;
            for (final value
                in (jsonRaw['connected_analyzers'] as Map<String, dynamic>)
                    .values) {
              final prediction =
                  PredictionModel.fromSerialized(value['configuration']);
              manager.mergePrediction(prediction);
              manager.addActive(prediction);
              liveAnalyses.predictions.addPrediction(prediction.name);
            }
            resetLiveAnalyses();
          }
        }
        break;
      case Command.getLastTrial:
        final jsonRaw = json.decode(utf8.decode(_responseData));
        if (jsonRaw != null) {
          lastTrialAnalogsData.appendFromJson(jsonRaw as Map<String, dynamic>);
        }
        break;
    }

    if (_commandAckCompleter?.isCompleted ?? false) _currentCommand = null;
    _responseCompleter!.complete();
  }

  Future<void> _receiveLiveAnalyses(List<int> raw) async {
    await _receiveLiveData(
      dataType: 'analyses',
      raw: raw,
      rawList: _rawLiveAnalysesList,
      getLastTimeStamp: ([DateTime? duration]) {
        if (duration != null) _lastLiveAnalysesTimestamp = duration;
        return _lastLiveAnalysesTimestamp;
      },
      getExpectedDataLength: ([int? value]) {
        if (value != null) {
          _expectedLiveAnalysesLength = value < 0 ? null : value;
        }
        return _expectedLiveAnalysesLength;
      },
      getLiveData: (bool isNew) {
        if (isNew) resetLiveAnalyses();
        return liveAnalyses;
      },
      getCompleter: (bool isNew) {
        if (isNew) _liveAnalysesDataCompleter = Completer();
        return _liveAnalysesDataCompleter;
      },
      liveDataTimeWindow: liveAnalysesTimeWindow,
      onNewData: _onNewLiveAnalyses,
    );
  }

  Future<void> _receiveLiveAnalogsData(List<int> raw) async {
    await _receiveLiveData(
      dataType: 'live data',
      raw: raw,
      rawList: _rawLiveAnalogsList,
      getLastTimeStamp: ([DateTime? duration]) {
        if (duration != null) _lastLiveAnalogsDataTimestamp = duration;
        return _lastLiveAnalogsDataTimestamp!;
      },
      getExpectedDataLength: ([int? value]) {
        if (value != null) {
          _expectedLiveAnalogsDataLength = value < 0 ? null : value;
        }
        return _expectedLiveAnalogsDataLength;
      },
      getLiveData: (bool isNew) {
        if (isNew) resetLiveAnalogsData();
        return liveAnalogsData;
      },
      getCompleter: (bool isNew) {
        if (isNew) _liveAnalogsDataCompleter = Completer();
        return _liveAnalogsDataCompleter;
      },
      liveDataTimeWindow: liveAnalogsDataTimeWindow,
      onNewData: _onNewLiveAnalogsData,
    );
  }

  Future<void> _receiveLiveData({
    required String dataType,
    required List<int> raw,
    required List<int> rawList,
    required DateTime? Function([DateTime? value]) getLastTimeStamp,
    required int? Function([int? value]) getExpectedDataLength,
    required Data Function(bool isNew) getLiveData,
    required Completer Function(bool isNew) getCompleter,
    required Duration liveDataTimeWindow,
    required Function()? onNewData,
  }) async {
    if (raw.isEmpty || !isInitialized) return;

    int? expectedDataLength = getExpectedDataLength();
    if (expectedDataLength == null) {
      getCompleter(true);
      try {
        getLastTimeStamp(_parseTimestampFromPacket(raw));
        expectedDataLength =
            getExpectedDataLength(_parseDataFromResponsePacket(raw));
      } catch (e) {
        _log.severe('Error while parsing $dataType: $e, resetting');
        resetLiveAnalogsData();
      }
      if (raw.length > _serverHeaderLength) {
        // If more data came at once, recursively call the function with the rest
        // of the data.
        _receiveLiveData(
            dataType: dataType,
            raw: raw.sublist(_serverHeaderLength),
            rawList: rawList,
            getLastTimeStamp: getLastTimeStamp,
            getExpectedDataLength: getExpectedDataLength,
            getLiveData: getLiveData,
            getCompleter: getCompleter,
            liveDataTimeWindow: liveDataTimeWindow,
            onNewData: onNewData);
      }
      return;
    }

    final completer = getCompleter(false);
    final lastTimeStamp = getLastTimeStamp();
    rawList.addAll(raw);
    if (rawList.length < expectedDataLength) {
      // Waiting for the rest of the live data
      return;
    } else if (rawList.length > expectedDataLength) {
      // We received the data for the next time frame, parse the current, then
      // call the function again with the rest of the data.
      final rawRemaining = rawList.sublist(expectedDataLength);
      rawList.removeRange(expectedDataLength, rawList.length);
      completer.future.then((_) => _receiveLiveData(
          dataType: dataType,
          raw: rawRemaining,
          rawList: rawList,
          getLastTimeStamp: getLastTimeStamp,
          getExpectedDataLength: getExpectedDataLength,
          getLiveData: getLiveData,
          getCompleter: getCompleter,
          liveDataTimeWindow: liveDataTimeWindow,
          onNewData: onNewData));
    }

    // Convert the data to a string (from json)
    try {
      final dataMap = json.decode(utf8.decode(rawList)) as Map<String, dynamic>;

      final liveData = getLiveData(false);
      if (liveData.isEmpty) {
        // If the live data were reset, we need to clear again with the current time stamp
        liveData.clear(initialTime: lastTimeStamp);
      }

      liveData.appendFromJson(dataMap);
      liveData.dropBefore(lastTimeStamp!.subtract(liveDataTimeWindow));
    } catch (e) {
      _log.severe('Error while parsing $dataType: $e, resetting');
      getLiveData(true);
    }

    rawList.clear();
    completer.complete();
    getExpectedDataLength(-1);

    if (onNewData != null) onNewData();
  }

  int _parse32bitsIntFromPacket(List<int> data) {
    if (data.length != 4) {
      throw ArgumentError('Data length must be 4 bytes');
    }
    final byteData = ByteData.sublistView(Uint8List.fromList(data));

    // Read as little-endian uint32
    return byteData.getUint32(0, Endian.little);
  }

  int _parse64bitsIntFromPacket(List<int> data) {
    if (data.length != 8) {
      throw ArgumentError('Data length must be 8 bytes');
    }
    final byteData = ByteData.sublistView(Uint8List.fromList(data));

    // Read as little-endian uint64
    return byteData.getUint64(0, Endian.little);
  }

  int _parseVersionFromPacket(List<int> data) {
    // Parse the version (4 bytes) from the packet data, starting from the 1st byte
    return _parse32bitsIntFromPacket(data.sublist(0, 4));
  }

  DateTime _parseTimestampFromPacket(List<int> data) {
    // Parse the timestamp (8 bytes) from the packet data, starting from the 5th byte
    int timestamp = _parse64bitsIntFromPacket(data.sublist(4, 12));
    return DateTime.fromMillisecondsSinceEpoch(timestamp);
  }

  int _parseDataFromResponsePacket(List<int> data) {
    // Parse the data length (4 bytes) from the packet data, starting from the 13th byte
    return _parse32bitsIntFromPacket(data.sublist(12, 16));
  }

  // Prepare the singleton
  static final NeurobioClient _instance = NeurobioClient._();
  NeurobioClient._();

  Future<Socket?> _connectToSocket({
    required int id,
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

        socket.add(Command.constructPacket(command: id));
        await socket.flush();

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

class NeurobioClientMock extends NeurobioClient {
  NeurobioClientMock._() : super._();

  static NeurobioClient get instance => _instance;
  static final NeurobioClient _instance = NeurobioClientMock._();

  bool _isMockInitialized = false;

  @override
  bool get isInitialized => _isMockInitialized;

  @override
  Future<void> initialize({
    String serverIp = 'localhost',
    int commandPort = 5000,
    int responsePort = 5001,
    int liveAnalogsDataPort = 5002,
    int liveAnalysesPort = 5003,
    int? nbOfRetries,
    required Function() onConnexionLost,
    required Function() onNewLiveAnalogsData,
    required Function() onNewLiveAnalyses,
  }) async {
    if (isInitialized) return;
    _isMockInitialized = true;

    _onNewLiveAnalogsData = onNewLiveAnalogsData;
    _onNewLiveAnalyses = onNewLiveAnalyses;
  }

  @override
  Future<void> _connectSockets({
    required String serverIp,
    required int commandPort,
    required int responsePort,
    required int liveAnalogsDataPort,
    required int liveAnalysesPort,
    required int? nbOfRetries,
    required Function()? onConnexionLost,
  }) async {}

  @override
  Future<void> _disconnectSockets() async {
    _isMockInitialized = false;
  }

  @override
  Future<Ack> _performSend(Command command) async {
    switch (command) {
      case Command.handshake:
      case Command.connectDelsysAnalog:
      case Command.connectDelsysEmg:
      case Command.connectMagstim:
      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.disconnectMagstim:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        break;
      case Command.getStates:
        _prepareResponseCompleter();
      case Command.getLastTrial:
        _prepareLastTrialResponse();
    }

    // Construct and send the command
    try {
      _currentCommand = command;
      _commandAckCompleter = Completer<Ack>();
      Future.delayed(const Duration(milliseconds: 500)).then((value) =>
          _receiveCommandAck([
            NeurobioClient.communicationProtocolVersion,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            1
          ]));
      return await _commandAckCompleter!.future;
    } on SocketException {
      _log.info('Connexion was closed by the server');
      disconnect();
      return Ack.nok;
    }
  }

  bool _hasRecordedMock = false;
  @override
  bool get hasRecorded => !isRecording && _hasRecordedMock;

  @override
  void _setFlagsFromCommand(Command command) {
    switch (command) {
      case Command.startRecording:
        _hasRecordedMock = true;
        break;
      case Command.handshake:
      case Command.getStates:
      case Command.connectDelsysAnalog:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.connectDelsysEmg:
      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
      case Command.stopRecording:
      case Command.connectMagstim:
      case Command.disconnectMagstim:
      case Command.getLastTrial:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        break;
    }

    super._setFlagsFromCommand(command);
  }
}

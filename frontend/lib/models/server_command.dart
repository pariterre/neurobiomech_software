import 'package:frontend/managers/neurobio_client.dart';

enum ServerCommand {
  handshake,
  getStates,
  connectDelsysAnalog,
  connectDelsysEmg,
  connectMagstim,
  zeroDelsysAnalog,
  zeroDelsysEmg,
  disconnectDelsysAnalog,
  disconnectDelsysEmg,
  disconnectMagstim,
  startRecording,
  stopRecording,
  getLastTrial,
  addAnalyzer,
  removeAnalyzer,
  failed,
  none;

  ///
  /// Value that corresponds to the command on the server
  int toInt() {
    switch (this) {
      case ServerCommand.handshake:
        return 0;
      case ServerCommand.getStates:
        return 1;
      case ServerCommand.connectDelsysAnalog:
        return 10;
      case ServerCommand.connectDelsysEmg:
        return 11;
      case ServerCommand.connectMagstim:
        return 12;
      case ServerCommand.zeroDelsysAnalog:
        return 40;
      case ServerCommand.zeroDelsysEmg:
        return 41;
      case ServerCommand.disconnectDelsysAnalog:
        return 20;
      case ServerCommand.disconnectDelsysEmg:
        return 21;
      case ServerCommand.disconnectMagstim:
        return 22;
      case ServerCommand.startRecording:
        return 30;
      case ServerCommand.stopRecording:
        return 31;
      case ServerCommand.getLastTrial:
        return 32;
      case ServerCommand.addAnalyzer:
        return 50;
      case ServerCommand.removeAnalyzer:
        return 51;
      case ServerCommand.failed:
        return 100;
      case ServerCommand.none:
        return 0xFFFFFFFF;
    }
  }

  ///
  /// Command that should not be called by the user (not part of the API)
  bool get isReserved {
    switch (this) {
      case ServerCommand.getStates:
      case ServerCommand.connectDelsysAnalog:
      case ServerCommand.connectDelsysEmg:
      case ServerCommand.connectMagstim:
      case ServerCommand.zeroDelsysAnalog:
      case ServerCommand.zeroDelsysEmg:
      case ServerCommand.disconnectDelsysAnalog:
      case ServerCommand.disconnectDelsysEmg:
      case ServerCommand.disconnectMagstim:
      case ServerCommand.startRecording:
      case ServerCommand.stopRecording:
      case ServerCommand.getLastTrial:
      case ServerCommand.addAnalyzer:
      case ServerCommand.removeAnalyzer:
        return false;
      case ServerCommand.handshake:
      case ServerCommand.failed:
      case ServerCommand.none:
        return true;
    }
  }

  bool get isImplemented {
    switch (this) {
      case ServerCommand.handshake:
      case ServerCommand.getStates:
      case ServerCommand.connectDelsysAnalog:
      case ServerCommand.connectDelsysEmg:
      case ServerCommand.zeroDelsysAnalog:
      case ServerCommand.zeroDelsysEmg:
      case ServerCommand.disconnectDelsysAnalog:
      case ServerCommand.disconnectDelsysEmg:
      case ServerCommand.startRecording:
      case ServerCommand.stopRecording:
      case ServerCommand.getLastTrial:
      case ServerCommand.addAnalyzer:
      case ServerCommand.removeAnalyzer:
      case ServerCommand.failed:
      case ServerCommand.none:
        return true;
      case ServerCommand.connectMagstim:
      case ServerCommand.disconnectMagstim:
        return false;
    }
  }

  bool get hasDataResponse {
    switch (this) {
      case ServerCommand.handshake:
      case ServerCommand.connectDelsysAnalog:
      case ServerCommand.connectDelsysEmg:
      case ServerCommand.connectMagstim:
      case ServerCommand.zeroDelsysAnalog:
      case ServerCommand.zeroDelsysEmg:
      case ServerCommand.disconnectDelsysAnalog:
      case ServerCommand.disconnectDelsysEmg:
      case ServerCommand.disconnectMagstim:
      case ServerCommand.startRecording:
      case ServerCommand.stopRecording:
      case ServerCommand.addAnalyzer:
      case ServerCommand.removeAnalyzer:
      case ServerCommand.failed:
      case ServerCommand.none:
        return false;
      case ServerCommand.getStates:
      case ServerCommand.getLastTrial:
        return true;
    }
  }

  List<int> toPacket() => constructPacket(command: toInt());

  static List<int> constructPacket({required int command}) {
    // Packets are exactly 8 bytes long, little-endian
    // - First 4 bytes are the version number
    // - Next 4 bytes are the command
    if (command < 0 || command > 0xFFFFFFFF) {
      throw ArgumentError('Command must be between 0 and 0xFFFFFFFF');
    }

    final protocolVersion = NeurobioClient.communicationProtocolVersion
        .toRadixString(16)
        .padLeft(8, '0');
    final commandRadix = command.toRadixString(16).padLeft(8, '0');

    // Split the strings by pairs of char
    final packet = <int>[];
    for (int i = 3; i >= 0; i--) {
      packet.add(
          int.parse(protocolVersion.substring(i * 2, i * 2 + 2), radix: 16));
    }
    for (int i = 3; i >= 0; i--) {
      packet
          .add(int.parse(commandRadix.substring(i * 2, i * 2 + 2), radix: 16));
    }
    return packet;
  }

  static ServerCommand fromInt(int value) {
    for (final command in ServerCommand.values) {
      if (command.toInt() == value) {
        return command;
      }
    }
    return ServerCommand.none;
  }
}

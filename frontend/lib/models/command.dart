import 'package:frontend/managers/neurobio_client.dart';

enum Command {
  handshake,
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
  removeAnalyzer;

  ///
  /// Value that corresponds to the command on the server
  int toInt() {
    switch (this) {
      case Command.handshake:
        return 0;
      case Command.connectDelsysAnalog:
        return 10;
      case Command.connectDelsysEmg:
        return 11;
      case Command.connectMagstim:
        return 12;
      case Command.zeroDelsysAnalog:
        return 40;
      case Command.zeroDelsysEmg:
        return 41;
      case Command.disconnectDelsysAnalog:
        return 20;
      case Command.disconnectDelsysEmg:
        return 21;
      case Command.disconnectMagstim:
        return 22;
      case Command.startRecording:
        return 30;
      case Command.stopRecording:
        return 31;
      case Command.getLastTrial:
        return 32;
      case Command.addAnalyzer:
        return 50;
      case Command.removeAnalyzer:
        return 51;
    }
  }

  ///
  /// Command that should not be called by the user (not part of the API)
  bool get isReserved {
    switch (this) {
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
      case Command.getLastTrial:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        return false;
      case Command.handshake:
        return true;
    }
  }

  bool get isImplemented {
    switch (this) {
      case Command.handshake:
      case Command.connectDelsysAnalog:
      case Command.connectDelsysEmg:
      case Command.zeroDelsysAnalog:
      case Command.zeroDelsysEmg:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.getLastTrial:
      case Command.addAnalyzer:
      case Command.removeAnalyzer:
        return true;
      case Command.connectMagstim:
      case Command.disconnectMagstim:
        return false;
    }
  }

  bool get hasDataResponse {
    switch (this) {
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
        return false;
      case Command.getLastTrial:
        return true;
    }
  }

  List<int> toPacket() => constructPacket(command: toInt());

  static List<int> constructPacket({required int command}) {
    // Packets are exactly 8 bytes long, little-endian
    // - First 4 bytes are the version number
    // - Next 4 bytes are the command

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
}

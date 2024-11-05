import 'package:flutter/services.dart';

enum Command {
  handshake,
  connectDelsysAnalog,
  connectDelsysEmg,
  connectMagstim,
  disconnectDelsysAnalog,
  disconnectDelsysEmg,
  disconnectMagstim,
  startRecording,
  stopRecording,
  getLastTrial;

  ///
  /// Value that corresponds to the command on the server
  int toInt() {
    switch (this) {
      case Command.handshake:
        return 0;
      case Command.connectDelsysAnalog:
        return 1;
      case Command.connectDelsysEmg:
        return 2;
      case Command.connectMagstim:
        return 3;
      case Command.disconnectDelsysAnalog:
        return 4;
      case Command.disconnectDelsysEmg:
        return 5;
      case Command.disconnectMagstim:
        return 6;
      case Command.startRecording:
        return 7;
      case Command.stopRecording:
        return 8;
      case Command.getLastTrial:
        return 9;
    }
  }

  ///
  /// Command that should not be called by the user (not part of the API)
  bool get isReserved {
    switch (this) {
      case Command.connectDelsysAnalog:
      case Command.connectDelsysEmg:
      case Command.connectMagstim:
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.disconnectMagstim:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.getLastTrial:
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
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.getLastTrial:
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
      case Command.disconnectDelsysAnalog:
      case Command.disconnectDelsysEmg:
      case Command.disconnectMagstim:
      case Command.startRecording:
      case Command.stopRecording:
        return false;
      case Command.getLastTrial:
        return true;
    }
  }

  Uint8List toPacket() {
    // Packets are exactly 8 bytes long, little-endian
    // - First 4 bytes are the version number
    // - Next 4 bytes are the command

    const protocolVersion = '0001';
    final command = toInt().toString().padLeft(4, '0');
    final packet = protocolVersion + command;

    // To little-endian
    return Uint8List.fromList([
      for (var i = 0; i < 8; i += 2)
        int.parse(packet.substring(i, i + 2), radix: 16)
    ]);
  }
}

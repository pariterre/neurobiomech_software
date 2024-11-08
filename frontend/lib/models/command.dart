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
        return 10;
      case Command.connectDelsysEmg:
        return 11;
      case Command.connectMagstim:
        return 12;
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

  List<int> toPacket() {
    // Packets are exactly 8 bytes long, little-endian
    // - First 4 bytes are the version number
    // - Next 4 bytes are the command

    final protocolVersion = 1.toRadixString(16).padLeft(8, '0');
    final command = toInt().toRadixString(16).padLeft(8, '0');

    // Split the strings by pairs of char
    final packet = <int>[];
    for (int i = 3; i >= 0; i--) {
      packet.add(
          int.parse(protocolVersion.substring(i * 2, i * 2 + 2), radix: 16));
    }
    for (int i = 3; i >= 0; i--) {
      packet.add(int.parse(command.substring(i * 2, i * 2 + 2), radix: 16));
    }
    return packet;
  }
}

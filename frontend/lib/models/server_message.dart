import 'dart:typed_data';

enum ServerMessage {
  ok,
  nok,
  listeningExtraData,
  sendingData,
  statesChanged;

  int toInt() {
    switch (this) {
      case ServerMessage.ok:
        return 0;
      case ServerMessage.nok:
        return 1;
      case ServerMessage.listeningExtraData:
        return 2;
      case ServerMessage.sendingData:
        return 10;
      case ServerMessage.statesChanged:
        return 20;
    }
  }

  static ServerMessage parse(List<int> packet) {
    final byteData =
        ByteData.sublistView(Uint8List.fromList(packet.sublist(8, 12)));

    // Read as little-endian uint32
    final valueAsInt = byteData.getUint32(0, Endian.little);
    for (final ack in ServerMessage.values) {
      if (ack.toInt() == valueAsInt) {
        return ack;
      }
    }
    return ServerMessage.nok;
  }
}

import 'dart:typed_data';

enum Ack {
  ok,
  nok,
  ready,
  sendingData;

  int toInt() {
    switch (this) {
      case Ack.ok:
        return 0;
      case Ack.nok:
        return 1;
      case Ack.ready:
        return 2;
      case Ack.sendingData:
        return 3;
    }
  }

  static Ack parse(List<int> packet) {
    final byteData =
        ByteData.sublistView(Uint8List.fromList(packet.sublist(8, 12)));

    // Read as little-endian uint32
    final valueAsInt = byteData.getUint32(0, Endian.little);
    for (final ack in Ack.values) {
      if (ack.toInt() == valueAsInt) {
        return ack;
      }
    }
    return Ack.nok;
  }
}

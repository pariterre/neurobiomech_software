import 'dart:typed_data';

enum Ack {
  nok,
  ok,
  statesChanged;

  int toInt() {
    switch (this) {
      case Ack.nok:
        return 0;
      case Ack.ok:
        return 1;
      case Ack.statesChanged:
        return 10;
    }
  }

  static Ack parse(List<int> packet) {
    final byteData =
        ByteData.sublistView(Uint8List.fromList(packet.sublist(12, 16)));

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

import 'dart:typed_data';

enum ServerDataType {
  states,
  fullTrial,
  liveData,
  liveAnalyses,
  none;

  int toInt() {
    switch (this) {
      case ServerDataType.states:
        return 0;
      case ServerDataType.fullTrial:
        return 1;
      case ServerDataType.liveData:
        return 10;
      case ServerDataType.liveAnalyses:
        return 11;
      case ServerDataType.none:
        return 0xFFFFFFFF;
    }
  }

  static ServerDataType parse(List<int> packet) {
    final byteData =
        ByteData.sublistView(Uint8List.fromList(packet.sublist(12, 16)));

    // Read as little-endian uint32
    final valueAsInt = byteData.getUint32(0, Endian.little);
    for (final dataType in ServerDataType.values) {
      if (dataType.toInt() == valueAsInt) {
        return dataType;
      }
    }
    return ServerDataType.none;
  }
}

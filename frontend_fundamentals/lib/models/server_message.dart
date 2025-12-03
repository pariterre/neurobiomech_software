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

  static ServerMessage fromInt(int value) {
    for (final ack in ServerMessage.values) {
      if (ack.toInt() == value) {
        return ack;
      }
    }
    return ServerMessage.nok;
  }
}

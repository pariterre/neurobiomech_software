enum Ack {
  nok,
  ok;

  int toInt() {
    switch (this) {
      case Ack.nok:
        return 0;
      case Ack.ok:
        return 1;
    }
  }

  static Ack parse(List<int> packet) {
    return packet[4] == Ack.ok.toInt() ? Ack.ok : Ack.nok;
  }
}

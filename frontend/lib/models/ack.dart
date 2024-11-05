enum Ack {
  ok,
  nok;

  static Ack parse(List<int> packet) {
    return packet[0] == 0 ? Ack.ok : Ack.nok;
  }
}

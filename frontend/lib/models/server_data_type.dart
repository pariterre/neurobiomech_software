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

  static ServerDataType fromInt(int value) {
    for (final dataType in ServerDataType.values) {
      if (dataType.toInt() == value) {
        return dataType;
      }
    }
    return ServerDataType.none;
  }
}

enum Command {
  startNidaq,
  stopNidaq,
  startRecording,
  stopRecording,
  stimulate,
  fetchData,
  plotData,
  saveData,
  quit,
  shutdown;

  ///
  /// Value that corresponds to the command on the server
  int toInt() {
    switch (this) {
      case Command.startNidaq:
        return 0;
      case Command.stopNidaq:
        return 1;
      case Command.startRecording:
        return 2;
      case Command.stopRecording:
        return 3;
      case Command.stimulate:
        return 4;
      case Command.fetchData:
        return 5;
      case Command.plotData:
        return 6;
      case Command.saveData:
        return 7;
      case Command.quit:
        return 8;
      case Command.shutdown:
        return 9;
    }
  }

  ///
  /// Command that should not be called by the user (not part of the API)
  bool get isReserved {
    switch (this) {
      case Command.startNidaq:
      case Command.stopNidaq:
      case Command.startRecording:
      case Command.stopRecording:
      case Command.stimulate:
      case Command.plotData:
      case Command.saveData:
      case Command.quit:
      case Command.shutdown:
        return false;
      case Command.fetchData:
        return true;
    }
  }
}

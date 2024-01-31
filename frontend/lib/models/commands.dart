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
}

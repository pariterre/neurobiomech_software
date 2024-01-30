enum Commands {
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
      case Commands.startRecording:
        return 1;
      case Commands.stopRecording:
        return 2;
      case Commands.stimulate:
        return 3;
      case Commands.fetchData:
        return 4;
      case Commands.plotData:
        return 5;
      case Commands.saveData:
        return 6;
      case Commands.quit:
        return 7;
      case Commands.shutdown:
        return 8;
      default:
        return -1;
    }
  }
}

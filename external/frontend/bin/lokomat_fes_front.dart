import 'dart:async';
import 'dart:convert';
import 'dart:io';

enum Command {
  startRecording,
  stopRecording,
  stimulate,
  plotData,
  saveData,
  quit;

  int toInt() {
    switch (this) {
      case Command.startRecording:
        return 1;
      case Command.stopRecording:
        return 2;
      case Command.stimulate:
        return 3;
      case Command.plotData:
        return 4;
      case Command.saveData:
        return 5;
      case Command.quit:
        return 6;
      default:
        return 0;
    }
  }
}

class Communication {
  Socket socket;
  String? lastAcknowledge;

  Communication._(this.socket) {
    socket.listen((data) => lastAcknowledge = utf8.decode(data));
  }

  static Future<Communication> connect(
      {String serverIp = 'localhost', int serverPort = 4042}) async {
    while (true) {
      try {
        final socket = await Socket.connect(serverIp, serverPort);
        return Communication._(socket);
      } on SocketException {
        print('Connection failed, retrying...');
        await Future.delayed(Duration(seconds: 1));
      }
    }
  }

  Future<bool> sendCommand(Command command, [List<String>? parameters]) async {
    // Construct and send the command
    lastAcknowledge = null;

    String message = "${command.toInt()}:";
    if (parameters != null) {
      message += parameters.join(',');
    }
    socket.write(message);
    try {
      await socket.flush();
    } on SocketException {
      print('Connexion was closed by the server');
      return false;
    }
    print('Sent command: $command');

    // Receive acknowledgment from the server
    while (lastAcknowledge == null) {
      await Future.delayed(Duration(milliseconds: 100));
    }
    return lastAcknowledge == "ACK - OK";
  }

  void close() {
    socket.close();
  }
}

void main() async {
  // Create a Communication instance
  print('Connecting to server...');
  Communication communication = await Communication.connect();
  print('Connected to server');

  // Send "start_device" request
  if (!(await communication.sendCommand(Command.startRecording))) {
    print('Communication was closed');
    return;
  }

  // Wait for 5 seconds
  await Future.delayed(Duration(seconds: 5));

  // Send "stimulate" request
  if (!(await communication.sendCommand(Command.stimulate, ['2.2']))) {
    print('Communication was closed');
    return;
  }

  // Wait for 5 seconds
  await Future.delayed(Duration(seconds: 5));

  // Send "stop_device" request
  if (!(await communication.sendCommand(Command.stopRecording))) {
    print('Communication was closed');
    return;
  }

  // Wait for 5 seconds
  await Future.delayed(Duration(seconds: 5));

  // Close the connection
  communication.close();
}

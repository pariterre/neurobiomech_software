// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/server_command.dart';

void main() {
  test('Command', () {
    const command = ServerCommand.values;
    assert(command.length == 14);

    assert(command[0] == ServerCommand.handshake);
    assert(command[1] == ServerCommand.connectDelsysAnalog);
    assert(command[2] == ServerCommand.connectDelsysEmg);
    assert(command[3] == ServerCommand.connectMagstim);
    assert(command[4] == ServerCommand.zeroDelsysAnalog);
    assert(command[5] == ServerCommand.zeroDelsysEmg);
    assert(command[6] == ServerCommand.disconnectDelsysAnalog);
    assert(command[7] == ServerCommand.disconnectDelsysEmg);
    assert(command[8] == ServerCommand.disconnectMagstim);
    assert(command[9] == ServerCommand.startRecording);
    assert(command[10] == ServerCommand.stopRecording);
    assert(command[11] == ServerCommand.getLastTrial);
    assert(command[12] == ServerCommand.addAnalyzer);
    assert(command[13] == ServerCommand.removeAnalyzer);

    assert(ServerCommand.handshake.toInt() == 0);
    assert(ServerCommand.connectDelsysAnalog.toInt() == 10);
    assert(ServerCommand.connectDelsysEmg.toInt() == 11);
    assert(ServerCommand.connectMagstim.toInt() == 12);
    assert(ServerCommand.zeroDelsysAnalog.toInt() == 40);
    assert(ServerCommand.zeroDelsysEmg.toInt() == 41);
    assert(ServerCommand.disconnectDelsysAnalog.toInt() == 20);
    assert(ServerCommand.disconnectDelsysEmg.toInt() == 21);
    assert(ServerCommand.disconnectMagstim.toInt() == 22);
    assert(ServerCommand.startRecording.toInt() == 30);
    assert(ServerCommand.stopRecording.toInt() == 31);
    assert(ServerCommand.getLastTrial.toInt() == 32);
    assert(ServerCommand.addAnalyzer.toInt() == 50);
    assert(ServerCommand.removeAnalyzer.toInt() == 51);

    assert(ServerCommand.handshake.isReserved);

    assert(!ServerCommand.connectDelsysAnalog.isReserved);
    assert(!ServerCommand.connectDelsysEmg.isReserved);
    assert(!ServerCommand.connectMagstim.isReserved);
    assert(!ServerCommand.zeroDelsysAnalog.isReserved);
    assert(!ServerCommand.zeroDelsysEmg.isReserved);
    assert(!ServerCommand.disconnectDelsysAnalog.isReserved);
    assert(!ServerCommand.disconnectDelsysEmg.isReserved);
    assert(!ServerCommand.disconnectMagstim.isReserved);
    assert(!ServerCommand.startRecording.isReserved);
    assert(!ServerCommand.stopRecording.isReserved);
    assert(!ServerCommand.getLastTrial.isReserved);
    assert(!ServerCommand.addAnalyzer.isReserved);
    assert(!ServerCommand.removeAnalyzer.isReserved);
  });
}

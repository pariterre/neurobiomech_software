// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/command.dart';

void main() {
  test('Command', () {
    const command = Command.values;
    assert(command.length == 10);

    assert(command[0] == Command.handshake);
    assert(command[1] == Command.connectDelsysAnalog);
    assert(command[2] == Command.connectDelsysEmg);
    assert(command[3] == Command.connectMagstim);
    assert(command[4] == Command.disconnectDelsysAnalog);
    assert(command[5] == Command.disconnectDelsysEmg);
    assert(command[6] == Command.disconnectMagstim);
    assert(command[7] == Command.startRecording);
    assert(command[8] == Command.stopRecording);
    assert(command[9] == Command.getLastTrial);

    assert(Command.handshake.toInt() == 0);
    assert(Command.connectDelsysAnalog.toInt() == 1);
    assert(Command.connectDelsysEmg.toInt() == 2);
    assert(Command.connectMagstim.toInt() == 3);
    assert(Command.disconnectDelsysAnalog.toInt() == 4);
    assert(Command.disconnectDelsysEmg.toInt() == 5);
    assert(Command.disconnectMagstim.toInt() == 6);
    assert(Command.startRecording.toInt() == 7);
    assert(Command.stopRecording.toInt() == 8);
    assert(Command.getLastTrial.toInt() == 9);

    assert(Command.handshake.isReserved);

    assert(!Command.connectDelsysAnalog.isReserved);
    assert(!Command.connectDelsysEmg.isReserved);
    assert(!Command.connectMagstim.isReserved);
    assert(!Command.disconnectDelsysAnalog.isReserved);
    assert(!Command.disconnectDelsysEmg.isReserved);
    assert(!Command.disconnectMagstim.isReserved);
    assert(!Command.startRecording.isReserved);
    assert(!Command.stopRecording.isReserved);
    assert(!Command.getLastTrial.isReserved);
  });
}

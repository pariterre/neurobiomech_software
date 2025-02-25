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
    assert(command.length == 14);

    assert(command[0] == Command.handshake);
    assert(command[1] == Command.connectDelsysAnalog);
    assert(command[2] == Command.connectDelsysEmg);
    assert(command[3] == Command.connectMagstim);
    assert(command[4] == Command.zeroDelsysAnalog);
    assert(command[5] == Command.zeroDelsysEmg);
    assert(command[6] == Command.disconnectDelsysAnalog);
    assert(command[7] == Command.disconnectDelsysEmg);
    assert(command[8] == Command.disconnectMagstim);
    assert(command[9] == Command.startRecording);
    assert(command[10] == Command.stopRecording);
    assert(command[11] == Command.getLastTrial);
    assert(command[12] == Command.addAnalyzer);
    assert(command[13] == Command.removeAnalyzer);

    assert(Command.handshake.toInt() == 0);
    assert(Command.connectDelsysAnalog.toInt() == 10);
    assert(Command.connectDelsysEmg.toInt() == 11);
    assert(Command.connectMagstim.toInt() == 12);
    assert(Command.zeroDelsysAnalog.toInt() == 40);
    assert(Command.zeroDelsysEmg.toInt() == 41);
    assert(Command.disconnectDelsysAnalog.toInt() == 20);
    assert(Command.disconnectDelsysEmg.toInt() == 21);
    assert(Command.disconnectMagstim.toInt() == 22);
    assert(Command.startRecording.toInt() == 30);
    assert(Command.stopRecording.toInt() == 31);
    assert(Command.getLastTrial.toInt() == 32);
    assert(Command.addAnalyzer.toInt() == 50);
    assert(Command.removeAnalyzer.toInt() == 51);

    assert(Command.handshake.isReserved);

    assert(!Command.connectDelsysAnalog.isReserved);
    assert(!Command.connectDelsysEmg.isReserved);
    assert(!Command.connectMagstim.isReserved);
    assert(!Command.zeroDelsysAnalog.isReserved);
    assert(!Command.zeroDelsysEmg.isReserved);
    assert(!Command.disconnectDelsysAnalog.isReserved);
    assert(!Command.disconnectDelsysEmg.isReserved);
    assert(!Command.disconnectMagstim.isReserved);
    assert(!Command.startRecording.isReserved);
    assert(!Command.stopRecording.isReserved);
    assert(!Command.getLastTrial.isReserved);
    assert(!Command.addAnalyzer.isReserved);
    assert(!Command.removeAnalyzer.isReserved);
  });
}

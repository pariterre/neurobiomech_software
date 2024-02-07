// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/commands.dart';

void main() {
  test('Command', () {
    const command = Command.values;
    assert(command.length == 15);
    assert(command[0] == Command.startNidaq);
    assert(command[1] == Command.stopNidaq);
    assert(command[2] == Command.startRecording);
    assert(command[3] == Command.stopRecording);
    assert(command[4] == Command.stimulate);
    assert(command[5] == Command.availableSchedules);
    assert(command[6] == Command.addSchedule);
    assert(command[7] == Command.getScheduled);
    assert(command[8] == Command.removeScheduled);
    assert(command[9] == Command.startFetchingData);
    assert(command[10] == Command.fetchData);
    assert(command[11] == Command.plotData);
    assert(command[12] == Command.saveData);
    assert(command[13] == Command.quit);
    assert(command[14] == Command.shutdown);

    assert(Command.startNidaq.toInt() == 0);
    assert(Command.stopNidaq.toInt() == 1);
    assert(Command.startRecording.toInt() == 2);
    assert(Command.stopRecording.toInt() == 3);
    assert(Command.stimulate.toInt() == 4);
    assert(Command.availableSchedules.toInt() == 5);
    assert(Command.addSchedule.toInt() == 6);
    assert(Command.getScheduled.toInt() == 7);
    assert(Command.removeScheduled.toInt() == 8);
    assert(Command.startFetchingData.toInt() == 9);
    assert(Command.fetchData.toInt() == 10);
    assert(Command.plotData.toInt() == 11);
    assert(Command.saveData.toInt() == 12);
    assert(Command.quit.toInt() == 13);
    assert(Command.shutdown.toInt() == 14);

    assert(!Command.startNidaq.isReserved);
    assert(!Command.stopNidaq.isReserved);
    assert(!Command.startRecording.isReserved);
    assert(!Command.stopRecording.isReserved);
    assert(!Command.stimulate.isReserved);
    assert(!Command.addSchedule.isReserved);
    assert(!Command.removeScheduled.isReserved);
    assert(!Command.plotData.isReserved);
    assert(!Command.saveData.isReserved);
    assert(!Command.quit.isReserved);
    assert(!Command.shutdown.isReserved);

    assert(Command.availableSchedules.isReserved);
    assert(Command.getScheduled.isReserved);
    assert(Command.startFetchingData.isReserved);
    assert(Command.fetchData.isReserved);
  });
}

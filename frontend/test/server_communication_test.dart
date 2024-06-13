// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/commands.dart';
import 'package:frontend/models/stimwalker_server_interface.dart';

void main() {
  test('Initialize server connexion', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    assert(connexion.isInitialized == false);

    connexion.initialize();
    assert(connexion.isInitialized);
    assert(connexion.isNidaqConnected == false);
    assert(connexion.isRecording == false);
    assert(connexion.isContinousDataActive == false);

    await connexion.send(Command.quit);
    assert(connexion.isInitialized == false);
  });

  test('Send command to server', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    assert(connexion.isNidaqConnected == false);
    assert(await connexion.send(Command.startNidaq) == false);
    assert(connexion.isNidaqConnected == false);

    connexion.initialize();
    assert(connexion.isNidaqConnected == false);

    assert(await connexion.send(Command.startNidaq));
    assert(connexion.isNidaqConnected);

    await connexion.send(Command.quit);
    assert(connexion.isNidaqConnected == false);
  });

  test('Cannot call reserved', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;

    await connexion.initialize();

    assert(await connexion.send(Command.fetchData) == false);
    assert(await connexion.send(Command.getScheduled) == false);
    assert(await connexion.send(Command.availableSchedules) == false);
    assert(await connexion.send(Command.startFetchingData) == false);

    await connexion.send(Command.quit);
  });

  test('Get available schedules', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    await connexion.initialize();

    final schedules = await connexion.fetchScheduledStimulation(
        command: Command.availableSchedules);
    assert(schedules.length == 3);
    assert(schedules[0].toString() == "schedule1");
    assert(schedules[1].toString() == "schedule2");
    assert(schedules[2].toString() == "schedule3");

    await connexion.send(Command.quit);
  });

  test('Get scheduled stimulation', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    connexion.initialize();

    final schedules = await connexion.fetchScheduledStimulation(
        command: Command.getScheduled);
    assert(schedules.length == 3);
    assert(schedules[0].toString() == "schedule1");
    assert(schedules[1].toString() == "schedule2");
    assert(schedules[2].toString() == "schedule3");
  });

  test('Automatic fetch data', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    connexion.initialize();

    bool callbackCalled = false;
    void onData() {
      callbackCalled = true;
    }

    // We need to start the niDaq before we can start the automatic data fetch
    assert(connexion.isNidaqConnected == false);
    assert(
        await connexion.startAutomaticDataFetch(onContinousDataReady: onData) ==
            false);
    assert(connexion.isContinousDataActive == false);

    assert(await connexion.send(Command.startNidaq));
    assert(connexion.isNidaqConnected);
    assert(
        await connexion.startAutomaticDataFetch(onContinousDataReady: onData));
    assert(connexion.isContinousDataActive);

    // Cannot start the automatic data fetch twice
    assert(
        await connexion.startAutomaticDataFetch(onContinousDataReady: onData) ==
            false);

    await Future.delayed(const Duration(milliseconds: 500));
    assert(connexion.isContinousDataActive);
    assert(callbackCalled);

    // Stop the automatic data fetch
    connexion.stopAutomaticDataFetch();
    await Future.delayed(const Duration(milliseconds: 500));
    assert(connexion.isContinousDataActive == false);

    callbackCalled = false;
    await Future.delayed(const Duration(milliseconds: 500));
    assert(!callbackCalled);

    await connexion.send(Command.quit);
  });

  test('Manage recording commands', () async {
    LokomatFesServerInterface connexion =
        LokomatFesServerInterfaceMock.newInstance;
    connexion.initialize();
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded == false);

    assert(await connexion.send(Command.startRecording));
    assert(connexion.isRecording);
    assert(connexion.hasRecorded);

    assert(await connexion.send(Command.stopRecording));
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded);

    await connexion.send(Command.quit);
  });
}

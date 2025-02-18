// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/models/neurobio_client.dart';

void main() {
  test('Initialize server connexion', () async {
    final connexion = NeurobioClientMock.instance;
    assert(connexion.isInitialized == false);

    connexion.initialize(
        onConnexionLost: () {}, onNewLiveData: () {}, onNewLiveAnalyses: () {});
    assert(connexion.isInitialized);
    assert(connexion.isConnectedToDelsysAnalog == false);
    assert(connexion.isConnectedToDelsysEmg == false);
    assert(connexion.isRecording == false);
    assert(connexion.isConnectedToLiveData == false);

    await connexion.disconnect();
    assert(connexion.isInitialized == false);
  });

  test('Send command to server', () async {
    final connexion = NeurobioClientMock.instance;

    assert(connexion.isConnectedToDelsysAnalog == false);
    assert(await connexion.send(Command.connectDelsysAnalog) == false);
    assert(connexion.isConnectedToDelsysAnalog == false);

    await connexion.initialize(
        onConnexionLost: () {}, onNewLiveData: () {}, onNewLiveAnalyses: () {});
    assert(connexion.isConnectedToDelsysAnalog == false);

    assert(await connexion.send(Command.connectDelsysAnalog));
    assert(connexion.isConnectedToDelsysAnalog);

    connexion.disconnect();
    assert(connexion.isConnectedToDelsysAnalog == false);
  });

  test('Cannot call reserved', () async {
    final connexion = NeurobioClientMock.instance;

    await connexion.initialize(
        onConnexionLost: () {}, onNewLiveData: () {}, onNewLiveAnalyses: () {});
    assert(await connexion.send(Command.handshake) == false);

    connexion.disconnect();
  });

  test('Manage recording commands', () async {
    final connexion = NeurobioClientMock.instance;
    connexion.initialize(
        onConnexionLost: () {}, onNewLiveData: () {}, onNewLiveAnalyses: () {});
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded == false);

    assert(await connexion.send(Command.startRecording));
    assert(connexion.isRecording);
    assert(connexion.hasRecorded == false);

    assert(await connexion.send(Command.stopRecording));
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded);

    connexion.disconnect();
  });
}

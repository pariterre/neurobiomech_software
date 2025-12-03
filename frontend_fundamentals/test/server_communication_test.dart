// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:frontend_fundamentals/managers/neurobio_client.dart';
import 'package:frontend_fundamentals/models/server_command.dart';
import 'package:flutter_test/flutter_test.dart';

void main() {
  test('Initialize server connexion', () async {
    final connexion = NeurobioClientMock.instance;
    assert(connexion.isConnected == false);

    await connexion.initialize();
    await connexion.connect();
    assert(connexion.isConnected);
    assert(connexion.isConnectedToDelsysAnalog == false);
    assert(connexion.isConnectedToDelsysEmg == false);
    assert(connexion.isRecording == false);
    assert(connexion.isConnectedToLiveAnalogsData == false);

    await connexion.disconnect();
    assert(connexion.isConnected == false);
  });

  test('Send command to server', () async {
    final connexion = NeurobioClientMock.instance;

    assert(connexion.isConnectedToDelsysAnalog == false);
    assert(await connexion.send(ServerCommand.connectDelsysAnalog) == false);
    assert(connexion.isConnectedToDelsysAnalog == false);

    await connexion.initialize();
    await connexion.connect();
    assert(connexion.isConnectedToDelsysAnalog == false);

    assert(await connexion.send(ServerCommand.connectDelsysAnalog));
    assert(connexion.isConnectedToDelsysAnalog);

    connexion.disconnect();
    assert(connexion.isConnectedToDelsysAnalog == false);
  });

  test('Cannot call reserved', () async {
    final connexion = NeurobioClientMock.instance;

    await connexion.initialize();
    await connexion.connect();
    assert(await connexion.send(ServerCommand.handshake) == false);

    connexion.disconnect();
  });

  test('Manage recording commands', () async {
    final connexion = NeurobioClientMock.instance;
    await connexion.initialize();
    await connexion.connect();
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded == false);

    assert(await connexion.send(ServerCommand.startRecording));
    assert(connexion.isRecording);
    assert(connexion.hasRecorded == false);

    assert(await connexion.send(ServerCommand.stopRecording));
    assert(connexion.isRecording == false);
    assert(connexion.hasRecorded);

    connexion.disconnect();
  });
}

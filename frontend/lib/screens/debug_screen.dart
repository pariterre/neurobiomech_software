import 'package:flutter/material.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/models/stimwalker_client.dart';

StimwalkerClient get _connexion => StimwalkerClient.instance;

class DebugScreen extends StatefulWidget {
  const DebugScreen({super.key});

  static const route = '/debug-screen';

  @override
  State<DebugScreen> createState() => _DebugScreenState();
}

class _DebugScreenState extends State<DebugScreen> {
  final _onlineGraphKey = GlobalKey<_OnlineGraphState>();

  bool _isBusy = false;
  bool get isServerConnected => _connexion.isInitialized;
  bool get canSendCommand => !_isBusy && isServerConnected;

  bool _showGraph = false;

  Future<void> _connectServer() async {
    setState(() => _isBusy = true);
    await _connexion.initialize();
    setState(() => _isBusy = false);
  }

  Future<void> _disconnectServer() async {
    setState(() => _isBusy = true);
    await _connexion.disconnect();
    _resetInternalStates();
  }

  void _resetInternalStates() {
    setState(() {
      _isBusy = false;
      _showGraph = false;
    });
  }

  Future<void> _connectDelsysAnalog() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.connectDelsysAnalog);
    setState(() => _isBusy = false);
  }

  Future<void> _connectDelsysEmg() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.connectDelsysEmg);
    _resetInternalStates();
  }

  Future<void> _disconnectDelsysAnalog() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.disconnectDelsysAnalog);
    _resetInternalStates();
  }

  Future<void> _disconnectDelsysEmg() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.disconnectDelsysEmg);
    _resetInternalStates();
  }

  Future<void> _startRecording() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.startRecording);
    setState(() => _isBusy = false);
  }

  Future<void> _stopRecording() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.stopRecording);
    setState(() => _isBusy = false);
  }

  Future<void> _getLastTrial() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.getLastTrial);
    setState(() => _isBusy = false);
  }

  Future<void> _showOnlineGraph() async {
    _connexion.resetLiveData();
    setState(() => _showGraph = true);
  }

  Future<void> _hideOnlineGraph() async {
    setState(() => _showGraph = false);
  }

  Widget _buildGraph() {
    if (!_showGraph) return const SizedBox();
    return _OnlineGraph(key: _onlineGraphKey);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const SizedBox(width: double.infinity),
              Text('Stimwalker controller',
                  style: Theme.of(context).textTheme.titleLarge),
              const SizedBox(height: 20),
              Text('Connection related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: _isBusy
                    ? null
                    : (_connexion.isInitialized
                        ? _disconnectServer
                        : _connectServer),
                child: Text(
                  _connexion.isInitialized ? 'Disconnect' : 'Connect',
                ),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                  onPressed: !_isBusy && _connexion.isInitialized
                      ? (_connexion.isConnectedToDelsysAnalog
                          ? _disconnectDelsysAnalog
                          : _connectDelsysAnalog)
                      : null,
                  child: Text(_connexion.isConnectedToDelsysAnalog
                      ? 'Disconnect Delsys Analog'
                      : 'Connect Delsys Analog')),
              const SizedBox(height: 12),
              ElevatedButton(
                  onPressed: !_isBusy && _connexion.isInitialized
                      ? (_connexion.isConnectedToDelsysEmg
                          ? _disconnectDelsysEmg
                          : _connectDelsysEmg)
                      : null,
                  child: Text(_connexion.isConnectedToDelsysEmg
                      ? 'Disconnect Delsys EMG'
                      : 'Connect Delsys EMG')),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: !_isBusy && _connexion.isInitialized
                    ? _disconnectServer
                    : null,
                child: const Text('Shutdown'),
              ),
              const SizedBox(height: 20),
              Text('Devices related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand
                    ? (_connexion.isRecording
                        ? _stopRecording
                        : _startRecording)
                    : null,
                child: Text(
                  _connexion.isRecording ? 'Stop recording' : 'Start recording',
                ),
              ),
              const SizedBox(height: 20),
              Text('Data related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand &&
                        _connexion.hasRecorded &&
                        !_connexion.isRecording
                    ? _getLastTrial
                    : null,
                child: const Text('Get last trial'),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand
                    ? _showGraph
                        ? _hideOnlineGraph
                        : _showOnlineGraph
                    : null,
                child: Text(
                    _showGraph ? 'Hide online graph' : 'Show online graph'),
              ),
              const SizedBox(height: 12),
              _buildGraph(),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: _showGraph ? _connexion.liveData.clear : null,
                child: const Text('Clear data'),
              ),
              const SizedBox(height: 12),
            ],
          ),
        ),
      ),
    );
  }
}

class _OnlineGraph extends StatefulWidget {
  const _OnlineGraph({super.key});

  @override
  State<_OnlineGraph> createState() => _OnlineGraphState();
}

class _OnlineGraphState extends State<_OnlineGraph> {
  @override
  Widget build(BuildContext context) {
    return Container();
  }
}

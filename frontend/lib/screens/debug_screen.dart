import 'package:flutter/material.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/stimwalker_client.dart';
import 'package:frontend/widgets/data_graph.dart';

StimwalkerClient get _connexion => StimwalkerClient.instance;

class DebugScreen extends StatefulWidget {
  const DebugScreen({super.key});

  static const route = '/debug-screen';

  @override
  State<DebugScreen> createState() => _DebugScreenState();
}

class _DebugScreenState extends State<DebugScreen> {
  bool _isBusy = false;
  bool get isServerConnected => _connexion.isInitialized;
  bool get canSendCommand => !_isBusy && isServerConnected;

  bool _showLastTrial = false;
  bool _showLiveData = false;

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
      _showLastTrial = false;
      _showLiveData = false;
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
    await _hideLastTrialGraph();
    setState(() => _isBusy = true);
    await _connexion.send(Command.startRecording);
    setState(() => _isBusy = false);
  }

  Future<void> _stopRecording() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.stopRecording);
    setState(() => _isBusy = false);
    _showLastTrialGraph();
  }

  Future<void> _showLastTrialGraph() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.getLastTrial);
    await _connexion.onDataArrived;
    setState(() {
      _isBusy = false;
      _showLastTrial = true;
    });
  }

  Future<void> _hideLastTrialGraph() async {
    setState(() => _showLastTrial = false);
  }

  Widget _buildLastTrialGraph() {
    if (!_showLastTrial) return const SizedBox();
    return DataGraph(data: _connexion.lastTrialData);
  }

  Future<void> _showLiveDataGraph() async {
    _connexion.resetLiveData();
    setState(() => _showLiveData = true);
  }

  Future<void> _hideLiveDataGraph() async {
    setState(() => _showLiveData = false);
  }

  Widget _buildLiveDataGraph() {
    if (!_showLiveData) return const SizedBox();
    // https://github.com/imaNNeo/fl_chart/blob/main/example/lib/presentation/samples/line/line_chart_sample10.dart
    return DataGraph(
        data: Data(t0: 0, analogChannelCount: 0, emgChannelCount: 0));
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
              const SizedBox(height: 12),
              _buildLastTrialGraph(),
              const SizedBox(height: 20),
              Text('Data related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand
                    ? _showLiveData
                        ? _hideLiveDataGraph
                        : _showLiveDataGraph
                    : null,
                child: Text(
                    _showLiveData ? 'Hide online graph' : 'Show online graph'),
              ),
              const SizedBox(height: 12),
              _buildLiveDataGraph(),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: _showLiveData ? _connexion.liveData.clear : null,
                child: const Text('Reset live data'),
              ),
              const SizedBox(height: 12),
            ],
          ),
        ),
      ),
    );
  }
}

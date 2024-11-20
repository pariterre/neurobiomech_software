import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/models/database_manager.dart';
import 'package:frontend/models/stimwalker_client.dart';
import 'package:frontend/widgets/data_graph.dart';
import 'package:frontend/widgets/save_trial_dialog.dart';

StimwalkerClient get _connexion => StimwalkerClient.instance;

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  static const route = '/main-screen';

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  final _liveGraphControllerAnalog = DataGraphController(
      data: _connexion.liveData, graphType: DataGraphType.analog);
  final _liveGraphControllerEmg = DataGraphController(
      data: _connexion.liveData, graphType: DataGraphType.emg);
  final _trialGraphControllerAnalog = DataGraphController(
      data: _connexion.lastTrialData, graphType: DataGraphType.analog);
  final _trialGraphControllerEmg = DataGraphController(
      data: _connexion.lastTrialData, graphType: DataGraphType.emg);

  bool _isBusy = false;
  bool get isServerConnected => _connexion.isInitialized;
  bool get canSendCommand => !_isBusy && isServerConnected;

  bool _showLastTrial = false;
  bool _showLiveData = false;

  Future<void> _connectServer() async {
    setState(() {
      _showLastTrial = false;
      _showLiveData = false;
      _isBusy = true;
    });
    await _connexion.initialize(
      onConnexionLost: () => setState(() {}),
      onNewLiveData: _onNewLiveData,
    );
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
      _showLastTrial = _connexion.lastTrialData.isNotEmpty;
    });
  }

  Future<void> _hideLastTrialGraph() async {
    setState(() => _showLastTrial = false);
  }

  Future<void> _saveTrial() async {
    final hasSaved = await showDialog(
        context: context, builder: (context) => const SaveTrialDialog());
    if (!hasSaved) return;

    _connexion.lastTrialData.toFile(DatabaseManager.instance.savePath);
  }

  Widget _buildLastTrialGraph() {
    if (!_showLastTrial) return const SizedBox();
    return Column(
      children: [
        ElevatedButton(
            style: ElevatedButton.styleFrom(
                backgroundColor: Colors.green, foregroundColor: Colors.white),
            onPressed: _saveTrial,
            child: const Text('Save trial')),
        if (_connexion.isConnectedToDelsysAnalog)
          DataGraph(controller: _trialGraphControllerAnalog),
        if (_connexion.isConnectedToDelsysEmg)
          DataGraph(controller: _trialGraphControllerEmg),
      ],
    );
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

    // TODO It cannot connect right now with both data type
    return Column(
      children: [
        SizedBox(
          width: 250,
          child: TextFormField(
            decoration: const InputDecoration(
              labelText: 'Live data duration',
              hintText: 'Enter the duration in seconds',
            ),
            initialValue: _connexion.liveDataTimeWindow.inSeconds.toString(),
            onChanged: (value) {
              final valueAsInt = int.tryParse(value);
              if (valueAsInt == null) return;
              _connexion.liveDataTimeWindow = Duration(seconds: valueAsInt);
            },
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
          ),
        ),
        if (_connexion.isConnectedToDelsysAnalog)
          DataGraph(controller: _liveGraphControllerAnalog),
        if (_connexion.isConnectedToDelsysEmg)
          DataGraph(controller: _liveGraphControllerEmg),
      ],
    );
  }

  void _onNewLiveData() {
    if (_showLiveData) {
      _liveGraphControllerAnalog.data = _connexion.liveData;
      _liveGraphControllerEmg.data = _connexion.liveData;
    }
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

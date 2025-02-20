import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/managers/database_manager.dart';
import 'package:frontend/managers/neurobio_client.dart';
import 'package:frontend/managers/predictions_manager.dart';
import 'package:frontend/models/command.dart';
import 'package:frontend/screens/predictions_dialog.dart';
import 'package:frontend/widgets/data_graph.dart';
import 'package:frontend/widgets/save_trial_dialog.dart';

NeurobioClient get _connexion => NeurobioClient.instance;

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  static const route = '/main-screen';

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  final _liveAnalogDataKey = GlobalKey();
  final _liveEmgDataKey = GlobalKey();
  final _liveAnalysesKey = GlobalKey();
  final _trialAnalogDataKey = GlobalKey();
  final _trialEmgDataKey = GlobalKey();

  final _liveGraphControllerAnalog = DataGraphController(
      data: _connexion.liveData, graphType: DataGraphType.analog);
  final _liveGraphControllerEmg = DataGraphController(
      data: _connexion.liveData, graphType: DataGraphType.emg);
  final _liveAnalysesGraphController = DataGraphController(
      data: _connexion.liveAnalyses, graphType: DataGraphType.predictions);

  final _trialGraphControllerAnalog = DataGraphController(
      data: _connexion.lastTrialData, graphType: DataGraphType.analog);
  final _trialGraphControllerEmg = DataGraphController(
      data: _connexion.lastTrialData, graphType: DataGraphType.emg);

  bool _isBusy = false;
  bool get isServerConnected => _connexion.isInitialized;
  bool get canSendCommand => !_isBusy && isServerConnected;

  bool _showLastTrial = false;
  bool _showLiveData = false;
  bool _showLiveAnalyses = false;

  Future<void> _connectServer() async {
    setState(() {
      _showLastTrial = false;
      _showLiveData = false;
      _showLiveAnalyses = false;
      _isBusy = true;
    });
    await _connexion.initialize(
      onConnexionLost: () => setState(() {}),
      onNewLiveData: _onNewLiveData,
      onNewLiveAnalyses: _onNewLiveAnalyses,
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
      _showLiveAnalyses = false;
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

  Future<void> _zeroDelsysAnalog() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.zeroDelsysAnalog);
    setState(() => _isBusy = false);
  }

  Future<void> _zeroDelsysEmg() async {
    setState(() => _isBusy = true);
    await _connexion.send(Command.zeroDelsysEmg);
    setState(() => _isBusy = false);
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
      _showLastTrial = _connexion.lastTrialData.notHasData;
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
    if (!_connexion.hasRecorded) return const SizedBox();
    return Column(
      children: [
        Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            _showLastTrial
                ? ElevatedButton(
                    onPressed: _hideLastTrialGraph,
                    child: const Text('Hide last trial'),
                  )
                : ElevatedButton(
                    onPressed: _showLastTrialGraph,
                    child: const Text('Show last trial'),
                  ),
            const SizedBox(width: 12),
            ElevatedButton(
                style: ElevatedButton.styleFrom(
                    backgroundColor: Colors.green,
                    foregroundColor: Colors.white),
                onPressed: _saveTrial,
                child: const Text('Save trial')),
          ],
        ),
        if (_showLastTrial)
          Column(
            children: [
              if (_connexion.isConnectedToDelsysAnalog)
                DataGraph(
                    key: _trialAnalogDataKey,
                    controller: _trialGraphControllerAnalog),
              if (_connexion.isConnectedToDelsysEmg)
                DataGraph(
                    key: _trialEmgDataKey,
                    controller: _trialGraphControllerEmg),
            ],
          )
      ],
    );
  }

  Future<void> _showLiveAnalysesGraph() async {
    _connexion.resetLiveAnalyses();
    setState(() => _showLiveAnalyses = true);
  }

  Future<void> _hideLiveAnalysesGraph() async {
    setState(() => _showLiveAnalyses = false);
  }

  Future<void> _showLiveAnalysesManagerDialog() async {
    final predictionManager = (await PredictionsManager.instance);
    if (!mounted) return;

    final predictions = await showDialog(
      barrierDismissible: false,
      context: context,
      builder: (context) => PredictionsDialog(
          predictions: predictionManager.predictions.toList()),
    );
    if (predictions == null) return;

    predictionManager.save(predictions);
  }

  Future<void> _showLiveDataGraph() async {
    _connexion.resetLiveData();
    setState(() => _showLiveData = true);
  }

  Future<void> _hideLiveDataGraph() async {
    setState(() => _showLiveData = false);
  }

  Widget _buildConnectDevice(String name,
      {required bool isConnected,
      required Function() onClickedConnect,
      required Function() onClickedDisconnect,
      required Function() onClickedZero}) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        isConnected
            ? ElevatedButton(
                onPressed: !_isBusy && _connexion.isInitialized
                    ? onClickedDisconnect
                    : null,
                child: Text('Disconnect $name'))
            : ElevatedButton(
                onPressed: !_isBusy && _connexion.isInitialized
                    ? onClickedConnect
                    : null,
                child: Text('Connect $name')),
        if (isConnected)
          Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              const SizedBox(width: 12),
              ElevatedButton(
                  onPressed: !_isBusy && _connexion.isInitialized
                      ? onClickedZero
                      : null,
                  child: const Text('Zéro')),
            ],
          )
      ],
    );
  }

  Widget _buildLiveAnalysesGraph() {
    if (!_showLiveAnalyses) return const SizedBox();

    return Column(
      children: [
        SizedBox(
          width: 250,
          child: TextFormField(
            decoration: const InputDecoration(
              labelText: 'Live data duration',
              hintText: 'Enter the duration in seconds',
            ),
            initialValue:
                _connexion.liveAnalysesTimeWindow.inSeconds.toString(),
            onChanged: (value) {
              final valueAsInt = int.tryParse(value);
              if (valueAsInt == null) return;
              _connexion.liveAnalysesTimeWindow = Duration(seconds: valueAsInt);
            },
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
          ),
        ),
        if (_connexion.isConnectedToLiveAnalyses)
          DataGraph(
              key: _liveAnalysesKey, controller: _liveAnalysesGraphController),
      ],
    );
  }

  void _onNewLiveAnalyses() {
    if (_showLiveAnalyses) {
      _liveAnalysesGraphController.data = _connexion.liveAnalyses;
    }
  }

  Widget _buildLiveDataGraph() {
    if (!_showLiveData) return const SizedBox();
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
          DataGraph(
              key: _liveAnalogDataKey, controller: _liveGraphControllerAnalog),
        if (_connexion.isConnectedToDelsysEmg)
          DataGraph(key: _liveEmgDataKey, controller: _liveGraphControllerEmg),
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
              Text('Neurobio controller',
                  style: Theme.of(context).textTheme.titleLarge),
              const SizedBox(height: 20),
              Text('Connection commands',
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
              _buildConnectDevice('Delsys Analog',
                  isConnected: _connexion.isConnectedToDelsysAnalog,
                  onClickedConnect: _connectDelsysAnalog,
                  onClickedDisconnect: _disconnectDelsysAnalog,
                  onClickedZero: _zeroDelsysAnalog),
              const SizedBox(height: 12),
              _buildConnectDevice('Delsys EMG',
                  isConnected: _connexion.isConnectedToDelsysEmg,
                  onClickedConnect: _connectDelsysEmg,
                  onClickedDisconnect: _disconnectDelsysEmg,
                  onClickedZero: _zeroDelsysEmg),
              const SizedBox(height: 20),
              Text('Devices commands',
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
              Text('Live analyses commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed:
                    canSendCommand ? _showLiveAnalysesManagerDialog : null,
                child: const Text('Manager'),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand
                    ? _showLiveAnalyses
                        ? _hideLiveAnalysesGraph
                        : _showLiveAnalysesGraph
                    : null,
                child: Text(_showLiveAnalyses
                    ? 'Hide live analyses graph'
                    : 'Show live analyses graph'),
              ),
              const SizedBox(height: 12),
              _buildLiveAnalysesGraph(),
              const SizedBox(height: 20),
              Text('Data commands',
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

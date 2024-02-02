import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/models/commands.dart';
import 'package:frontend/models/lokomat_fes_server_interface.dart';

class DebugScreen extends StatefulWidget {
  const DebugScreen({super.key});

  static const route = '/debug-screen';

  @override
  State<DebugScreen> createState() => _DebugScreenState();
}

class _DebugScreenState extends State<DebugScreen> {
  final _onlineGraphKey = GlobalKey<_OnlineGraphState>();
  final _stimulationDurationTextController = TextEditingController();
  final _stimulationAmplitudeTextController = TextEditingController();
  final _saveTextController = TextEditingController();

  bool _isBusy = false;
  bool get isServerConnected =>
      LokomatFesServerInterface.instance.isInitialized;
  bool get canSendOfflineCommand => !_isBusy && isServerConnected;

  bool get canSendOnlineCommand =>
      !_isBusy &&
      isServerConnected &&
      LokomatFesServerInterface.instance.isNidaqConnected;
  bool get canManipulateData =>
      canSendOfflineCommand &&
      LokomatFesServerInterface.instance.hasRecorded &&
      !LokomatFesServerInterface.instance.isRecording;
  bool _showingGraph = false;
  bool get canSave => _saveTextController.text.isNotEmpty && canManipulateData;

  Future<void> _connectServer() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.initialize();
    setState(() => _isBusy = false);
  }

  Future<void> _disconnectServer() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.quit);
    _resetInternalStates();
  }

  void _resetInternalStates() {
    _stimulationDurationTextController.clear();
    _stimulationAmplitudeTextController.clear();
    _saveTextController.clear();

    setState(() {
      _isBusy = false;
      _showingGraph = false;
    });
  }

  Future<void> _connectNidaq() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.startNidaq);
    setState(() => _isBusy = false);
  }

  Future<void> _disconnectNidaq() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.stopNidaq);
    _resetInternalStates();
  }

  Future<void> _shutdown() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.shutdown);
    _resetInternalStates();
  }

  Future<void> _startRecording() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.startRecording);
    setState(() => _isBusy = false);
  }

  Future<void> _stopRecording() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.stopRecording);
    setState(() => _isBusy = false);
  }

  Future<void> _stimulate(
      String durationAsString, String amplitudeAsString) async {
    final duration = double.tryParse(durationAsString);
    if (duration == null) return;
    final parameters = [duration.toString()];
    String snackbarText = 'Stimulating for $duration seconds';

    final amplitude = double.tryParse(amplitudeAsString);
    if (amplitude != null) {
      parameters.add(amplitude.toString());
      snackbarText += ' with $amplitude mV';
    } else {
      snackbarText += ' with default amplitude';
    }

    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.stimulate, parameters: parameters);
    setState(() => _isBusy = false);

    // Notify the user that the data is being saved
    if (!mounted) return;
    ScaffoldMessenger.of(context)
        .showSnackBar(SnackBar(content: Text(snackbarText)));
  }

  Future<void> _saveData(String path) async {
    setState(() => _isBusy = true);

    // add .pkl extension if not present
    if (!path.endsWith('.pkl')) path += '.pkl';

    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.saveData, parameters: [path]);
    setState(() => _isBusy = false);

    // Notify the user that the data is being saved
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(
          'Data saved to ${_saveTextController.text}...',
        ),
      ),
    );
  }

  Future<void> _showOnlineGraph() async {
    final connexion = LokomatFesServerInterface.instance;
    connexion.startAutomaticDataFetch(
        onContinousDataReady: () =>
            _onlineGraphKey.currentState?.setState(() {}));
    setState(() => _showingGraph = true);
  }

  Future<void> _hideOnlineGraph() async {
    final connexion = LokomatFesServerInterface.instance;
    connexion.stopAutomaticDataFetch();
    setState(() => _showingGraph = false);
  }

  Future<void> _plotData() async {
    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.plotData);
    setState(() => _isBusy = false);
  }

  Widget _buildStimulation() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      mainAxisSize: MainAxisSize.min,
      children: [
        const Text('Stimulation'),
        Row(
          mainAxisSize: MainAxisSize.min,
          children: [
            SizedBox(
              width: 150,
              child: TextFormField(
                controller: _stimulationDurationTextController,
                enabled: canSendOnlineCommand,
                decoration: const InputDecoration(
                  labelText: 'Duration (s)',
                  border: OutlineInputBorder(),
                ),
                inputFormatters: [
                  FilteringTextInputFormatter.allow(RegExp(r'(^\d*\.?\d{0,2})'))
                ],
              ),
            ),
            const SizedBox(width: 12),
            SizedBox(
              width: 150,
              child: TextFormField(
                controller: _stimulationAmplitudeTextController,
                enabled: canSendOnlineCommand,
                decoration: const InputDecoration(
                  labelText: 'Amplitude (mV)',
                  border: OutlineInputBorder(),
                ),
                inputFormatters: [
                  FilteringTextInputFormatter.allow(RegExp(r'(^\d*)'))
                ],
              ),
            ),
            const SizedBox(width: 12),
            IconButton(
                onPressed: canSendOnlineCommand
                    ? () => _stimulate(
                          _stimulationDurationTextController.text,
                          _stimulationAmplitudeTextController.text,
                        )
                    : null,
                icon: const Icon(Icons.flash_on)),
          ],
        ),
      ],
    );
  }

  Widget _buildGraph() {
    final nidaq = LokomatFesServerInterface.instance.continousData?.nidaq;
    if (nidaq == null || !_showingGraph) {
      return const SizedBox();
    }

    return _OnlineGraph(key: _onlineGraphKey);
  }

  @override
  Widget build(BuildContext context) {
    final connexion = LokomatFesServerInterface.instance;

    return Scaffold(
      body: Center(
        child: SingleChildScrollView(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const SizedBox(width: double.infinity),
              Text('Lokomat_fes debugger',
                  style: Theme.of(context).textTheme.titleLarge),
              const SizedBox(height: 20),
              Text('Connection related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: _isBusy
                    ? null
                    : (connexion.isInitialized
                        ? _disconnectServer
                        : _connectServer),
                child: Text(
                  connexion.isInitialized ? 'Disconnect' : 'Connect',
                ),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                  onPressed: !_isBusy &&
                          LokomatFesServerInterface.instance.isInitialized
                      ? (connexion.isNidaqConnected
                          ? _disconnectNidaq
                          : _connectNidaq)
                      : null,
                  child: Text(connexion.isNidaqConnected
                      ? 'Disconnect Nidaq'
                      : 'Connect Nidaq')),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed:
                    !_isBusy && LokomatFesServerInterface.instance.isInitialized
                        ? _shutdown
                        : null,
                child: const Text('Shutdown'),
              ),
              const SizedBox(height: 20),
              Text('Devices related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendOnlineCommand
                    ? (connexion.isRecording ? _stopRecording : _startRecording)
                    : null,
                child: Text(
                  connexion.isRecording ? 'Stop recording' : 'Start recording',
                ),
              ),
              const SizedBox(height: 12),
              _buildStimulation(),
              const SizedBox(height: 20),
              Text('Data related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendOnlineCommand
                    ? _showingGraph
                        ? _hideOnlineGraph
                        : _showOnlineGraph
                    : null,
                child: Text(
                    _showingGraph ? 'Hide online graph' : 'Show online graph'),
              ),
              const SizedBox(height: 12),
              _buildGraph(),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: _showingGraph
                    ? LokomatFesServerInterface.instance.continousData?.clear
                    : null,
                child: const Text('Clear data'),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed:
                    canManipulateData && !_showingGraph ? (_plotData) : null,
                child: const Text('Plot on server'),
              ),
              const SizedBox(height: 12),
              const SizedBox(width: 20),
              SizedBox(
                width: 300,
                child: TextField(
                  controller: _saveTextController,
                  enabled: canManipulateData,
                  decoration: InputDecoration(
                    suffixIcon: IconButton(
                        onPressed: canSendOfflineCommand && canSave
                            ? () => _saveData(_saveTextController.text)
                            : null,
                        icon: const Icon(Icons.save)),
                    labelText: 'File name',
                    hintText: 'Enter a file name (*.pkl)',
                    border: const OutlineInputBorder(),
                  ),
                  onChanged: (value) => setState(() {}),
                ),
              ),
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
    final nidaq = LokomatFesServerInterface.instance.continousData!.nidaq;
    final rehastim = LokomatFesServerInterface.instance.continousData!.rehastim;

    final minTime = nidaq.t.isEmpty ? 0 : nidaq.t.last - 10;

    return SizedBox(
      width: 300,
      height: 200,
      child: LineChart(
        LineChartData(
          lineBarsData: [
            LineChartBarData(
                spots: nidaq.t.asMap().entries.map((index) {
              if (nidaq.t[index.key] < minTime) return FlSpot.nullSpot;

              final t = nidaq.t[index.key] - nidaq.t0;
              final y = nidaq.data[0][index.key];
              return FlSpot(t, y);
            }).toList()),
            ...rehastim.data.map((data) {
              if (data.t + data.duration < minTime) return LineChartBarData();

              return LineChartBarData(
                color: Colors.red,
                spots: [
                  FlSpot(data.t - rehastim.t0, data.channels[0].amplitude),
                  FlSpot(data.t - rehastim.t0 + data.duration,
                      data.channels[0].amplitude),
                ],
              );
            }).toList(),
          ],
        ),
      ),
    );
  }
}

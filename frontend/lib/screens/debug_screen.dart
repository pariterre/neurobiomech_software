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
  final _stimulationTextController = TextEditingController();
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
    _stimulationTextController.clear();
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
    setState(() => _isBusy = false);
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

  Future<void> _stimulate(String durationAsString) async {
    final duration = double.tryParse(durationAsString);
    if (duration == null) return;

    setState(() => _isBusy = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Command.stimulate, parameters: [duration.toString()]);
    setState(() => _isBusy = false);

    // Notify the user that the data is being saved
    if (!mounted) return;
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(
          'Stimulating for $duration seconds...',
        ),
      ),
    );
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
    connexion.startAutomaticDataFetch();
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
              SizedBox(
                width: 300,
                child: TextFormField(
                  controller: _stimulationTextController,
                  enabled: LokomatFesServerInterface.instance.isRecording,
                  decoration: InputDecoration(
                    suffixIcon: IconButton(
                        onPressed: canSendOnlineCommand
                            ? () => _stimulate(_stimulationTextController.text)
                            : null,
                        icon: const Icon(Icons.flash_on)),
                    labelText: 'Stimulation duration (s)',
                    hintText: 'Enter a duration in seconds',
                    border: const OutlineInputBorder(),
                  ),
                  inputFormatters: [
                    FilteringTextInputFormatter.allow(
                        RegExp(r'(^\d*\.?\d{0,2})'))
                  ],
                ),
              ),
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

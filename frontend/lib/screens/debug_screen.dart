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

  bool _isCommunicating = false;
  bool get canSendCommand =>
      !_isCommunicating && LokomatFesServerInterface.instance.isInitialized;
  bool get canManipulateData =>
      LokomatFesServerInterface.instance.hasRecorded &&
      !LokomatFesServerInterface.instance.isRecording;
  bool get canSave => _saveTextController.text.isNotEmpty && canManipulateData;

  Future<void> _connect() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.initialize();
    setState(() => _isCommunicating = false);
  }

  Future<void> _disconnect() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.quit);
    setState(() => _isCommunicating = false);
  }

  Future<void> _shutdown() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.shutdown);
    setState(() => _isCommunicating = false);
  }

  Future<void> _startRecording() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.startRecording);
    setState(() => _isCommunicating = false);
  }

  Future<void> _stopRecording() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.stopRecording);
    setState(() => _isCommunicating = false);
  }

  Future<void> _stimulate(String durationAsString) async {
    final duration = double.tryParse(durationAsString);
    if (duration == null) return;

    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.stimulate, [duration.toString()]);
    setState(() => _isCommunicating = false);

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
    setState(() => _isCommunicating = true);

    // add .pkl extension if not present
    if (!path.endsWith('.pkl')) path += '.pkl';

    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.saveData, [path]);
    setState(() => _isCommunicating = false);

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

  Future<void> _plotData() async {
    setState(() => _isCommunicating = true);
    final connexion = LokomatFesServerInterface.instance;
    await connexion.send(Commands.plotData);
    setState(() => _isCommunicating = false);
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
                onPressed: _isCommunicating
                    ? null
                    : (connexion.isInitialized ? _disconnect : _connect),
                child: Text(
                  connexion.isInitialized ? 'Disconnect' : 'Connect',
                ),
              ),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand ? _shutdown : null,
                child: const Text('Shutdown'),
              ),
              const SizedBox(height: 20),
              Text('Devices related commands',
                  style: Theme.of(context).textTheme.titleMedium),
              const SizedBox(height: 12),
              ElevatedButton(
                onPressed: canSendCommand
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
                        onPressed: canSendCommand
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
                onPressed: canSendCommand ? _plotData : null,
                child: const Text('Plot'),
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
                        onPressed: canSendCommand && canSave
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

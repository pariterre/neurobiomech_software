import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/managers/predictions_manager.dart';
import 'package:frontend/models/prediction_model.dart';

class DecimalInputFormatter extends TextInputFormatter {
  @override
  TextEditingValue formatEditUpdate(
      TextEditingValue oldValue, TextEditingValue newValue) {
    if (newValue.text.isEmpty) {
      return newValue;
    }

    final text = newValue.text;

    // Ensure only valid float format (digits and at most one dot)
    if (RegExp(r'^\d*\.?\d*$').hasMatch(text)) {
      return newValue;
    }

    // Reject the change if it's invalid
    return oldValue;
  }
}

class PredictionsDialog extends StatelessWidget {
  const PredictionsDialog({super.key});

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
        future: PredictionsManager.instance,
        builder: (context, snapshot) {
          if (snapshot.connectionState != ConnectionState.done) {
            return const Center(child: CircularProgressIndicator());
          }
          final predictions = snapshot.data?.predictions;
          if (predictions == null || predictions.isEmpty) {
            return const Center(child: Text('No live analyses'));
          }

          return AlertDialog(
            title: const Text('Live Analyses'),
            content: SingleChildScrollView(
              child: Column(
                children: predictions
                    .map((predictionModel) =>
                        _PredictionModelTile(predictionModel: predictionModel))
                    .toList(),
              ),
            ),
            actions: [
              TextButton(
                onPressed: () {
                  Navigator.of(context).pop();
                },
                child: const Text('Close'),
              ),
            ],
          );
        });
  }
}

class _PredictionModelTile extends StatelessWidget {
  const _PredictionModelTile({required this.predictionModel});

  final PredictionModel predictionModel;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        TextField(
          controller: TextEditingController(text: predictionModel.name),
          decoration: const InputDecoration(labelText: 'Name'),
        ),
        const SizedBox(height: 8),
        const Text('Analyzer type'),
        DropdownButton(
            value: predictionModel.analyzer,
            items: PredictionAnalyzers.values
                .map((e) => DropdownMenuItem(
                      value: e,
                      child: Text(e.toString()),
                    ))
                .toList(),
            onChanged: (value) {}),
        const SizedBox(height: 8),
        TextField(
          controller: TextEditingController(
            text: predictionModel.learningRate.toString(),
          ),
          keyboardType: TextInputType.number,
          inputFormatters: [DecimalInputFormatter()],
          decoration: const InputDecoration(labelText: 'Learning rate'),
        ),
        const SizedBox(height: 8),
        const Text('Time reference device'),
        DropdownButton(
            value: predictionModel.timeReferenceDevice,
            items: PredictionDevices.values
                .map((e) =>
                    DropdownMenuItem(value: e, child: Text(e.toString())))
                .toList(),
            onChanged: (value) {}),
        const SizedBox(height: 8),
        ...predictionModel.events
            .asMap()
            .keys
            .map((index) => Column(
                  children: [
                    _buildEventTile(index, predictionModel.events[index]),
                    const SizedBox(height: 16),
                  ],
                ))
            .toList(),
      ],
    );
  }

  Widget _buildEventTile(int index, PredictionEvent event) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('Event ${index + 1}'),
        const SizedBox(height: 8),
        Padding(
            padding: const EdgeInsets.only(left: 16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                TextField(
                  controller: TextEditingController(text: event.name),
                  decoration: const InputDecoration(labelText: 'Name'),
                ),
                const SizedBox(height: 8),
                const Text('Previous event name'),
                DropdownButton(
                  value: event.previousEventName,
                  items: predictionModel.events
                      .map((e) =>
                          DropdownMenuItem(value: e.name, child: Text(e.name)))
                      .toList(),
                  onChanged: (value) {},
                ),
                const SizedBox(height: 8),
                TextField(
                  controller: TextEditingController(
                    text: event.duration.inMilliseconds.toString(),
                  ),
                  keyboardType: TextInputType.number,
                  inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                  decoration: const InputDecoration(labelText: 'Duration (ms)'),
                ),
                const SizedBox(height: 8),
                ...event.startWhen.map((e) => _buildStartWhenTile(e)).toList(),
              ],
            )),
      ],
    );
  }

  Widget _buildStartWhenTile(PredictionStartWhen startWhen) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Start when'),
        const SizedBox(height: 8),
        const Text('Type'),
        DropdownButton(
          value: startWhen.runtimeType,
          items: const [
            DropdownMenuItem(
                value: PredictionStartWhenDirection,
                child: Text('Direction of device channel')),
            DropdownMenuItem(
                value: PredictionStartWhenThreshold,
                child: Text('Threshold of device channel')),
          ],
          onChanged: (value) {},
        ),
        const SizedBox(height: 8),
        switch (startWhen.runtimeType) {
          PredictionStartWhenDirection => _buildStartWhenDirectionTile(
              startWhen as PredictionStartWhenDirection),
          PredictionStartWhenThreshold => _buildStartWhenThresholdTile(
              startWhen as PredictionStartWhenThreshold),
          _ => throw UnimplementedError(
              'Unsupported PredictionStartWhen type: ${startWhen.runtimeType}'),
        },
      ],
    );
  }

  Widget _buildStartWhenDirectionTile(PredictionStartWhenDirection startWhen) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Device'),
        DropdownButton(
          value: startWhen.device,
          items: PredictionDevices.values
              .map((e) => DropdownMenuItem(value: e, child: Text(e.toString())))
              .toList(),
          onChanged: (value) {},
        ),
        const SizedBox(height: 8),
        TextField(
          controller: TextEditingController(text: startWhen.channel.toString()),
          keyboardType: TextInputType.number,
          inputFormatters: [FilteringTextInputFormatter.digitsOnly],
          decoration: const InputDecoration(labelText: 'Channel'),
        ),
        const SizedBox(height: 8),
        const Text('Direction'),
        DropdownButton(
          value: startWhen.direction,
          items: PredictionDirections.values
              .map((e) => DropdownMenuItem(value: e, child: Text(e.toString())))
              .toList(),
          onChanged: (value) {},
        ),
      ],
    );
  }

  Widget _buildStartWhenThresholdTile(PredictionStartWhenThreshold startWhen) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const Text('Device'),
        DropdownButton(
          value: startWhen.device,
          items: PredictionDevices.values
              .map((e) => DropdownMenuItem(value: e, child: Text(e.toString())))
              .toList(),
          onChanged: (value) {},
        ),
        const SizedBox(height: 8),
        TextField(
          controller: TextEditingController(text: startWhen.channel.toString()),
          keyboardType: TextInputType.number,
          inputFormatters: [FilteringTextInputFormatter.digitsOnly],
          decoration: const InputDecoration(labelText: 'Channel'),
        ),
        const SizedBox(height: 8),
        TextField(
          controller: TextEditingController(text: startWhen.value.toString()),
          keyboardType: TextInputType.number,
          inputFormatters: [DecimalInputFormatter()],
          decoration: const InputDecoration(labelText: 'Threshold'),
        ),
      ],
    );
  }
}

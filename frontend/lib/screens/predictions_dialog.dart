import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/models/prediction_model.dart';
import 'package:frontend/widgets/animated_expanding_card.dart';

class DecimalInputFormatter extends TextInputFormatter {
  @override
  TextEditingValue formatEditUpdate(
      TextEditingValue oldValue, TextEditingValue newValue) {
    if (newValue.text.isEmpty) return newValue;

    // Ensure only valid positive and negative float format (digits and at most one dot)
    final text = newValue.text;
    if (RegExp(r'^-?\d*\.?\d*$').hasMatch(text)) return newValue;

    // Reject the change if it's invalid
    return oldValue;
  }
}

class PredictionsDialog extends StatefulWidget {
  const PredictionsDialog({super.key, required this.predictions});

  final List<PredictionModel> predictions;

  @override
  State<PredictionsDialog> createState() => _PredictionsDialogState();
}

class _PredictionsDialogState extends State<PredictionsDialog> {
  void _onPredictionChanged(PredictionModel model, int modelIndex) {
    setState(() {
      widget.predictions[modelIndex] = model;
    });
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('Live Analyses'),
      content: SingleChildScrollView(
        child: Column(children: [
          ...widget.predictions.asMap().keys.map(
                (modelIndex) => _PredictionModelTile(
                  model: widget.predictions[modelIndex],
                  modelIndex: modelIndex,
                  onChanged: (newModel) =>
                      _onPredictionChanged(newModel, modelIndex),
                  onDeleted: () {
                    setState(() {
                      widget.predictions.removeAt(modelIndex);
                    });
                  },
                ),
              ),
          _AddNewTile(
              label: 'Add new analysis',
              labelStyle: Theme.of(context).textTheme.titleMedium,
              onTap: () {
                setState(() => widget.predictions.add(PredictionModel.empty(
                    name: Iterable<int>.generate(widget.predictions.length + 2)
                        .map((i) => 'New analysis ${i + 1}')
                        .firstWhere((name) =>
                            !widget.predictions.any((e) => e.name == name)))));
              }),
        ]),
      ),
      actions: [
        TextButton(
            onPressed: () {
              Navigator.of(context).pop();
            },
            child: const Text('Cancel')),
        ElevatedButton(
          onPressed: () {
            Navigator.of(context).pop(widget.predictions);
          },
          child: const Text('Save'),
        ),
      ],
    );
  }
}

class _PredictionModelTile extends StatelessWidget {
  const _PredictionModelTile(
      {required this.model,
      required this.modelIndex,
      required this.onChanged,
      required this.onDeleted});

  final int modelIndex;
  final PredictionModel model;
  final Function(PredictionModel) onChanged;
  final Function() onDeleted;

  @override
  Widget build(BuildContext context) {
    return AnimatedExpandingCard(
      header: Text('Analyses ${modelIndex + 1} (${model.name})',
          style: Theme.of(context).textTheme.titleMedium),
      child: Padding(
        padding: const EdgeInsets.only(left: 12.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Flexible(
                  child: _SaveOnFocusLostTextField(
                    label: 'Name',
                    initialValue: model.name,
                    onChanged: (value) =>
                        onChanged(model.copyWith(name: value)),
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.only(right: 8.0),
                  child: IconButton(
                    icon: const Icon(Icons.delete, color: Colors.red, size: 20),
                    onPressed: onDeleted,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 8),
            _LabelledDropdownButton(
                label: 'Analyzer type',
                value: model.analyzer,
                items: PredictionAnalyzers.values,
                onChanged: (value) =>
                    onChanged(model.copyWith(analyzer: value))),
            const SizedBox(height: 8),
            Row(
              children: [
                SizedBox(
                  width: 100,
                  child: _SaveOnFocusLostTextField(
                    label: 'Learning rate',
                    initialValue: model.learningRate.toString(),
                    onChanged: (value) => onChanged(
                        model.copyWith(learningRate: double.parse(value))),
                    keyboardType: TextInputType.number,
                    inputFormatters: [DecimalInputFormatter()],
                  ),
                ),
                const SizedBox(width: 24),
                _LabelledDropdownButton(
                    label: 'Time reference device',
                    value: model.timeReferenceDevice,
                    items: PredictionDevices.values,
                    onChanged: (newValue) => onChanged(
                        model.copyWith(timeReferenceDevice: newValue))),
              ],
            ),
            const SizedBox(height: 8),
            ...model.events
                .asMap()
                .keys
                .map((eventIndex) => Padding(
                      padding: const EdgeInsets.only(bottom: 8.0),
                      child: _buildEventTile(
                        event: model.events[eventIndex],
                        eventIndex: eventIndex,
                        onChanged: (event) {
                          final events = model.events.toList();

                          final oldName = events[eventIndex].name;
                          events[eventIndex] = event;

                          // Change the name of the "previous event"
                          for (int i = 0; i < events.length; i++) {
                            if (events[i].previousEventName == oldName) {
                              events[i] = events[i]
                                  .copyWith(previousEventName: event.name);
                            }
                          }

                          // Copy the new event to the list and send it
                          onChanged(model.copyWith(events: events));
                        },
                        onDeleted: () {
                          final events = model.events.toList();
                          final oldName = events[eventIndex].name;

                          // Change the name of the "previous event"
                          final newOldNameIndex =
                              (eventIndex - 1) % events.length;
                          for (int i = 0; i < events.length; i++) {
                            if (events[i].previousEventName == oldName) {
                              events[i] = events[i].copyWith(
                                  previousEventName:
                                      events[newOldNameIndex].name);
                            }
                          }

                          events.removeAt(eventIndex);
                          onChanged(model.copyWith(events: events));
                        },
                      ),
                    ))
                .toList(),
            _AddNewTile(
              label: 'Add new event',
              onTap: () {
                onChanged(model.copyWith(
                  events: [
                    ...model.events,
                    PredictionEvent.empty(
                        name: Iterable<int>.generate(model.events.length + 2)
                            .map((i) => 'New event ${i + 1}')
                            .firstWhere((name) =>
                                !model.events.any((e) => e.name == name))),
                  ],
                ));
              },
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildEventTile({
    required PredictionEvent event,
    required int eventIndex,
    required Function(PredictionEvent) onChanged,
    required Function() onDeleted,
  }) {
    return AnimatedExpandingCard(
      header: Text('Event ${eventIndex + 1} (${event.name})'),
      child: Padding(
        padding: const EdgeInsets.only(left: 12.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Flexible(
                  child: _SaveOnFocusLostTextField(
                    label: 'Name',
                    initialValue: event.name,
                    onChanged: (value) =>
                        onChanged(event.copyWith(name: value)),
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.only(right: 8.0),
                  child: IconButton(
                    icon: const Icon(Icons.delete, color: Colors.red, size: 20),
                    onPressed: onDeleted,
                  ),
                ),
              ],
            ),
            const SizedBox(height: 8),
            Row(
              children: [
                SizedBox(
                  width: 100,
                  child: _SaveOnFocusLostTextField(
                    label: 'Duration (ms)',
                    initialValue: event.duration.inMilliseconds.toString(),
                    keyboardType: TextInputType.number,
                    inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                    onChanged: (value) => onChanged(event.copyWith(
                        duration: Duration(milliseconds: int.parse(value)))),
                  ),
                ),
                const SizedBox(width: 24),
                _LabelledDropdownButton(
                  label: 'Previous event name',
                  value: event.previousEventName,
                  items: model.events.map((e) => e.name).toList(),
                  onChanged: (value) =>
                      onChanged(event.copyWith(previousEventName: value)),
                ),
              ],
            ),
            const SizedBox(height: 8),
            ...event.startWhen
                .asMap()
                .keys
                .map((startWhenIndex) => _buildStartWhenTile(
                      startWhen: event.startWhen[startWhenIndex],
                      startWhenIndex: startWhenIndex,
                      onChanged: (newStartWhen) {
                        final startWhens = event.startWhen.toList();
                        startWhens[startWhenIndex] = newStartWhen;
                        onChanged(event.copyWith(startWhen: startWhens));
                      },
                      onDeleted: () {
                        final startWhens = event.startWhen.toList();
                        startWhens.removeAt(startWhenIndex);
                        onChanged(event.copyWith(startWhen: startWhens));
                      },
                    ))
                .toList(),
            _AddNewTile(
              label: 'Add new start condition',
              onTap: () {
                onChanged(event.copyWith(
                  startWhen: [
                    ...event.startWhen,
                    PredictionStartWhenThreshold.empty()
                  ],
                ));
              },
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildStartWhenTile({
    required PredictionStartWhen startWhen,
    required int startWhenIndex,
    required Function(PredictionStartWhen) onChanged,
    required Function() onDeleted,
  }) {
    return Padding(
      padding: const EdgeInsets.only(bottom: 2.0),
      child: AnimatedExpandingCard(
        header: Text('Start condition ${startWhenIndex + 1}'),
        child: Padding(
          padding: const EdgeInsets.only(left: 12.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceBetween,
                children: [
                  _LabelledDropdownButton(
                    label: 'Type',
                    value: startWhen.type,
                    items: PredictionStartWhenTypes.values,
                    onChanged: (value) {
                      onChanged(switch (value) {
                        PredictionStartWhenTypes.threshold =>
                          PredictionStartWhenThreshold.empty(),
                        PredictionStartWhenTypes.direction =>
                          PredictionStartWhenDirection.empty(),
                      });
                    },
                  ),
                  Padding(
                    padding: const EdgeInsets.only(right: 8.0),
                    child: IconButton(
                      icon:
                          const Icon(Icons.delete, color: Colors.red, size: 20),
                      onPressed: onDeleted,
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              switch (startWhen.runtimeType) {
                PredictionStartWhenDirection => _buildStartWhenDirectionTile(
                    startWhen: startWhen as PredictionStartWhenDirection,
                    onChanged: onChanged),
                PredictionStartWhenThreshold => _buildStartWhenThresholdTile(
                    startWhen: startWhen as PredictionStartWhenThreshold,
                    onChanged: onChanged),
                _ => throw UnimplementedError(
                    'Unsupported PredictionStartWhen type: ${startWhen.runtimeType}'),
              },
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildStartWhenDirectionTile(
      {required PredictionStartWhenDirection startWhen,
      required Function(PredictionStartWhen) onChanged}) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            _LabelledDropdownButton(
              label: 'Device',
              value: startWhen.device,
              items: PredictionDevices.values,
              onChanged: (value) =>
                  onChanged(startWhen.copyWith(device: value)),
            ),
            const SizedBox(width: 24),
            SizedBox(
              width: 100,
              child: _SaveOnFocusLostTextField(
                label: 'Channel',
                initialValue: startWhen.channel.toString(),
                keyboardType: TextInputType.number,
                inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                onChanged: (value) =>
                    onChanged(startWhen.copyWith(channel: int.parse(value))),
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        _LabelledDropdownButton(
          label: 'Direction',
          value: startWhen.direction,
          items: PredictionDirections.values,
          onChanged: (value) => onChanged(startWhen.copyWith(direction: value)),
        ),
      ],
    );
  }

  Widget _buildStartWhenThresholdTile(
      {required PredictionStartWhenThreshold startWhen,
      required Function(PredictionStartWhen) onChanged}) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Row(
          children: [
            _LabelledDropdownButton(
              label: 'Device',
              value: startWhen.device,
              items: PredictionDevices.values,
              onChanged: (value) =>
                  onChanged(startWhen.copyWith(device: value)),
            ),
            const SizedBox(width: 24),
            SizedBox(
              width: 100,
              child: _SaveOnFocusLostTextField(
                label: 'Channel',
                initialValue: startWhen.channel.toString(),
                keyboardType: TextInputType.number,
                inputFormatters: [FilteringTextInputFormatter.digitsOnly],
                onChanged: (value) =>
                    onChanged(startWhen.copyWith(channel: int.parse(value))),
              ),
            ),
          ],
        ),
        const SizedBox(height: 8),
        Row(
          children: [
            _LabelledDropdownButton(
              label: 'Comparator',
              value: startWhen.comparator,
              items: PredictionComparators.values,
              onChanged: (value) =>
                  onChanged(startWhen.copyWith(comparator: value)),
            ),
            const SizedBox(width: 24),
            SizedBox(
              width: 100,
              child: _SaveOnFocusLostTextField(
                  label: 'Threshold',
                  initialValue: startWhen.value.toString(),
                  keyboardType: TextInputType.number,
                  inputFormatters: [DecimalInputFormatter()],
                  onChanged: (value) => onChanged(
                      startWhen.copyWith(value: double.parse(value)))),
            ),
          ],
        ),
      ],
    );
  }
}

class _AddNewTile extends StatelessWidget {
  const _AddNewTile(
      {required this.label, this.labelStyle, required this.onTap});

  final String label;
  final TextStyle? labelStyle;
  final Function() onTap;

  @override
  Widget build(BuildContext context) {
    return InkWell(
      onTap: onTap,
      child: Padding(
        padding: const EdgeInsets.only(top: 2.0, bottom: 2.0),
        child:
            Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [
          Text(label, style: labelStyle),
          Padding(
            padding: const EdgeInsets.only(right: 20.0),
            child: Container(
                decoration: BoxDecoration(
                  color: Colors.green,
                  borderRadius: BorderRadius.circular(25),
                ),
                child: const SizedBox(
                    width: 16,
                    height: 16,
                    child: Icon(Icons.add, color: Colors.white, size: 16))),
          ),
        ]),
      ),
    );
  }
}

class _SaveOnFocusLostTextField extends StatelessWidget {
  const _SaveOnFocusLostTextField(
      {required this.label,
      required this.initialValue,
      this.keyboardType,
      this.inputFormatters,
      required this.onChanged});

  final String label;
  final String initialValue;
  final TextInputType? keyboardType;
  final List<TextInputFormatter>? inputFormatters;
  final Function(String) onChanged;

  @override
  Widget build(BuildContext context) {
    final controller = TextEditingController(text: initialValue);
    final focusNode = FocusNode();
    focusNode.addListener(() {
      if (!focusNode.hasFocus) onChanged(controller.text);
    });

    focusNode.addListener(() {
      if (!focusNode.hasFocus) {
        onChanged(controller.text);
      }
    });

    return TextField(
      controller: controller,
      focusNode: focusNode,
      keyboardType: keyboardType,
      inputFormatters: inputFormatters,
      decoration: InputDecoration(labelText: label),
    );
  }
}

class _LabelledDropdownButton<T> extends StatelessWidget {
  const _LabelledDropdownButton(
      {required this.label,
      required this.value,
      required this.items,
      required this.onChanged});

  final String label;
  final T value;
  final List<T> items;
  final Function(T) onChanged;

  @override
  Widget build(BuildContext context) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(label, style: Theme.of(context).textTheme.labelSmall),
        DropdownButton(
            value: value,
            items: items
                .map((e) =>
                    DropdownMenuItem(value: e, child: Text(e.toString())))
                .toList(),
            onChanged: (value) => onChanged(value as T)),
      ],
    );
  }
}

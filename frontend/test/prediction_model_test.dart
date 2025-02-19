import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/prediction_model.dart';

void main() {
  test('PredictionComparator enum Constructor', () {
    expect(PredictionComparators.fromString('>'),
        PredictionComparators.greaterThan);
    expect(PredictionComparators.fromString('>='),
        PredictionComparators.greaterThanOrEqual);
    expect(
        PredictionComparators.fromString('<'), PredictionComparators.lessThan);
    expect(PredictionComparators.fromString('<='),
        PredictionComparators.lessThanOrEqual);
    expect(PredictionComparators.fromString('=='), PredictionComparators.equal);
    expect(
        PredictionComparators.fromString('!='), PredictionComparators.notEqual);
  });

  test('PredictionComparator enum values', () {
    expect(PredictionComparators.values.length, 6);

    expect(PredictionComparators.greaterThan.toString(), '>');
    expect(PredictionComparators.greaterThanOrEqual.toString(), '>=');
    expect(PredictionComparators.lessThan.toString(), '<');
    expect(PredictionComparators.lessThanOrEqual.toString(), '<=');
    expect(PredictionComparators.equal.toString(), '==');
    expect(PredictionComparators.notEqual.toString(), '!=');
  });

  test('PredictionDirections enum Constructor', () {
    expect(PredictionDirections.fromString('positive'),
        PredictionDirections.positive);
    expect(PredictionDirections.fromString('negative'),
        PredictionDirections.negative);
  });

  test('PredictionDirections enum values', () {
    expect(PredictionDirections.values.length, 2);

    expect(PredictionDirections.positive.toString(), 'positive');
    expect(PredictionDirections.negative.toString(), 'negative');
  });

  test('PredictionAnalyzers enum Constructor', () {
    expect(PredictionAnalyzers.fromString('cyclic_from_analogs'),
        PredictionAnalyzers.cyclicFromAnalogs);
  });

  test('PredictionAnalyzers enum values', () {
    expect(PredictionAnalyzers.values.length, 1);

    expect(PredictionAnalyzers.cyclicFromAnalogs.toString(),
        'cyclic_from_analogs');
  });

  test('PredictionDevices enum Constructor', () {
    expect(PredictionDevices.fromString('DelsysAnalogDataCollector'),
        PredictionDevices.delsysAnalogDataCollector);
    expect(PredictionDevices.fromString('DelsysEmgDataCollector'),
        PredictionDevices.delsysEmgDataCollector);
  });

  test('PredictionDevices enum values', () {
    expect(PredictionDevices.values.length, 2);

    expect(PredictionDevices.delsysAnalogDataCollector.toString(),
        'DelsysAnalogDataCollector');
    expect(PredictionDevices.delsysEmgDataCollector.toString(),
        'DelsysEmgDataCollector');
  });

  test('PredictionStartWhenThreshold', () {
    final event = PredictionStartWhenThreshold.fromJson({
      'device': 'DelsysAnalogDataCollector',
      'channel': 0,
      'comparator': '>',
      'value': 0.5
    });

    expect(event.device, PredictionDevices.delsysAnalogDataCollector);
    expect(event.channel, 0);
    expect(event.comparator, PredictionComparators.greaterThan);
    expect(event.value, 0.5);

    final serialized = event.serialize();
    expect(serialized['device'], 'DelsysAnalogDataCollector');
    expect(serialized['channel'], 0);
    expect(serialized['comparator'], '>');
    expect(serialized['value'], 0.5);

    final created = PredictionStartWhen.factory(serialized);
    expect(created, isA<PredictionStartWhenThreshold>());
    final createAsThreshold = created as PredictionStartWhenThreshold;
    expect(
        createAsThreshold.device, PredictionDevices.delsysAnalogDataCollector);
    expect(createAsThreshold.channel, 0);
    expect(createAsThreshold.comparator, PredictionComparators.greaterThan);
    expect(createAsThreshold.value, 0.5);
  });

  test('PredictionStartWhenDirection', () {
    final event = PredictionStartWhenDirection.fromJson({
      'device': 'DelsysAnalogDataCollector',
      'channel': 0,
      'direction': 'positive'
    });

    expect(event.device, PredictionDevices.delsysAnalogDataCollector);
    expect(event.channel, 0);
    expect(event.direction, PredictionDirections.positive);

    final serialized = event.serialize();
    expect(serialized['device'], 'DelsysAnalogDataCollector');
    expect(serialized['channel'], 0);
    expect(serialized['direction'], 'positive');

    final created = PredictionStartWhen.factory(serialized);
    expect(created, isA<PredictionStartWhenDirection>());
    final createAsDirection = created as PredictionStartWhenDirection;
    expect(
        createAsDirection.device, PredictionDevices.delsysAnalogDataCollector);
    expect(createAsDirection.channel, 0);
    expect(createAsDirection.direction, PredictionDirections.positive);
  });

  test('PredictionEvent', () {
    final event = PredictionEvent(
        name: 'First event',
        previousEventName: 'Last event',
        duration: Duration.zero,
        startWhen: [
          PredictionStartWhenThreshold(
              device: PredictionDevices.delsysAnalogDataCollector,
              channel: 0,
              comparator: PredictionComparators.greaterThan,
              value: 0.5),
          PredictionStartWhenDirection(
              device: PredictionDevices.delsysAnalogDataCollector,
              channel: 1,
              direction: PredictionDirections.positive)
        ]);

    final serialized = event.serialize();
    expect(serialized['name'], 'First event');
    expect(serialized['previous'], 'Last event');
    expect(serialized['start_when'].length, 2);
    expect(serialized['start_when'][0]['device'], 'DelsysAnalogDataCollector');
    expect(serialized['start_when'][0]['channel'], 0);
    expect(serialized['start_when'][0]['comparator'], '>');
    expect(serialized['start_when'][0]['value'], 0.5);
    expect(serialized['start_when'][1]['device'], 'DelsysAnalogDataCollector');
    expect(serialized['start_when'][1]['channel'], 1);
    expect(serialized['start_when'][1]['direction'], 'positive');

    final created = PredictionEvent.fromSerialized(serialized, Duration.zero);
    expect(created.name, 'First event');
    expect(created.previousEventName, 'Last event');
    expect(created.startWhen.length, 2);
    expect(created.startWhen[0], isA<PredictionStartWhenThreshold>());
    {
      final startWhen = created.startWhen[0] as PredictionStartWhenThreshold;
      expect(startWhen.device, PredictionDevices.delsysAnalogDataCollector);
      expect(startWhen.channel, 0);
      expect(startWhen.comparator, PredictionComparators.greaterThan);
      expect(startWhen.value, 0.5);
    }
    expect(created.startWhen[1], isA<PredictionStartWhenDirection>());
    {
      final startWhen = created.startWhen[1] as PredictionStartWhenDirection;
      expect(startWhen.device, PredictionDevices.delsysAnalogDataCollector);
      expect(startWhen.channel, 1);
      expect(startWhen.direction, PredictionDirections.positive);
    }
  });

  test('PredictionModel', () {
    final model = PredictionModel(
        name: 'Test Model',
        analyzer: PredictionAnalyzers.cyclicFromAnalogs,
        timeReferenceDevice: PredictionDevices.delsysAnalogDataCollector,
        learningRate: 0.1,
        events: [
          PredictionEvent(
              name: 'First event',
              previousEventName: 'Last event',
              duration: const Duration(milliseconds: 123),
              startWhen: [
                PredictionStartWhenThreshold(
                    device: PredictionDevices.delsysAnalogDataCollector,
                    channel: 0,
                    comparator: PredictionComparators.greaterThan,
                    value: 0.5),
                PredictionStartWhenDirection(
                    device: PredictionDevices.delsysAnalogDataCollector,
                    channel: 1,
                    direction: PredictionDirections.positive)
              ]),
        ]);

    final serialized = model.serialize();
    expect(serialized['name'], 'Test Model');
    expect(serialized['analyzer_type'], 'cyclic_from_analogs');
    expect(serialized['time_reference_device'], 'DelsysAnalogDataCollector');
    expect(serialized['learning_rate'], 0.1);
    expect(serialized['initial_phase_durations'], [123]);
    expect(serialized['events'].length, 1);
    expect(serialized['events'][0]['name'], 'First event');
    expect(serialized['events'][0]['previous'], 'Last event');
    expect(serialized['events'][0]['start_when'].length, 2);
    expect(serialized['events'][0]['start_when'][0]['device'],
        'DelsysAnalogDataCollector');
    expect(serialized['events'][0]['start_when'][0]['channel'], 0);
    expect(serialized['events'][0]['start_when'][0]['comparator'], '>');
    expect(serialized['events'][0]['start_when'][0]['value'], 0.5);
    expect(serialized['events'][0]['start_when'][1]['device'],
        'DelsysAnalogDataCollector');
    expect(serialized['events'][0]['start_when'][1]['channel'], 1);
    expect(serialized['events'][0]['start_when'][1]['direction'], 'positive');

    final created = PredictionModel.fromSerialized(serialized);
    expect(created.name, 'Test Model');
    expect(created.analyzer, PredictionAnalyzers.cyclicFromAnalogs);
    expect(created.timeReferenceDevice,
        PredictionDevices.delsysAnalogDataCollector);
    expect(created.learningRate, 0.1);
    expect(created.events.length, 1);
    expect(created.events[0].name, 'First event');
    expect(created.events[0].previousEventName, 'Last event');
    expect(created.events[0].startWhen.length, 2);
    expect(created.events[0].startWhen[0], isA<PredictionStartWhenThreshold>());
    {
      final event =
          created.events[0].startWhen[0] as PredictionStartWhenThreshold;
      expect(event.device, PredictionDevices.delsysAnalogDataCollector);
      expect(event.channel, 0);
      expect(event.comparator, PredictionComparators.greaterThan);
      expect(event.value, 0.5);
    }
    expect(created.events[0].startWhen[1], isA<PredictionStartWhenDirection>());
    {
      final event =
          created.events[0].startWhen[1] as PredictionStartWhenDirection;
      expect(event.device, PredictionDevices.delsysAnalogDataCollector);
      expect(event.channel, 1);
      expect(event.direction, PredictionDirections.positive);
    }
  });
}

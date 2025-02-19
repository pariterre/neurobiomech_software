abstract class PredictionStartWhen {
  Map<String, dynamic> serialize();

  static PredictionStartWhen factory(Map<String, dynamic> json) {
    final type = json['type'];
    switch (type) {
      case 'threshold':
        return PredictionStartWhenThreshold.fromJson(json);
      case 'direction':
        return PredictionStartWhenDirection.fromJson(json);
      default:
        throw Exception('Unknown event type: $type');
    }
  }
}

enum PredictionComparators {
  greaterThan,
  greaterThanOrEqual,
  lessThan,
  lessThanOrEqual,
  equal,
  notEqual;

  @override
  String toString() {
    switch (this) {
      case greaterThan:
        return '>';
      case greaterThanOrEqual:
        return '>=';
      case lessThan:
        return '<';
      case lessThanOrEqual:
        return '<=';
      case equal:
        return '==';
      case notEqual:
        return '!=';
    }
  }

  static PredictionComparators fromString(String s) =>
      PredictionComparators.values.firstWhere((e) => e.toString() == s,
          orElse: () {
        throw Exception('Unknown comparator: $s');
      });
}

class PredictionStartWhenThreshold implements PredictionStartWhen {
  final PredictionDevices device;
  final int channel;
  final PredictionComparators comparator;
  final double value;

  PredictionStartWhenThreshold({
    required this.device,
    required this.channel,
    required this.comparator,
    required this.value,
  });

  @override
  Map<String, dynamic> serialize() {
    return {
      'type': 'threshold',
      'device': device.toString(),
      'channel': channel,
      'comparator': comparator.toString(),
      'value': value,
    };
  }

  @override
  PredictionStartWhenThreshold.fromJson(Map<String, dynamic> json)
      : device = PredictionDevices.fromString(json['device']),
        channel = json['channel'],
        comparator = PredictionComparators.fromString(json['comparator']),
        value = json['value'];
}

enum PredictionDirections {
  positive,
  negative;

  @override
  String toString() {
    switch (this) {
      case positive:
        return 'positive';
      case negative:
        return 'negative';
    }
  }

  static PredictionDirections fromString(String s) =>
      PredictionDirections.values.firstWhere((e) => e.toString() == s,
          orElse: () {
        throw Exception('Unknown direction: $s');
      });
}

class PredictionStartWhenDirection implements PredictionStartWhen {
  final PredictionDevices device;
  final int channel;
  final PredictionDirections direction;

  PredictionStartWhenDirection({
    required this.device,
    required this.channel,
    required this.direction,
  });

  @override
  Map<String, dynamic> serialize() {
    return {
      'type': 'direction',
      'device': device.toString(),
      'channel': channel,
      'direction': direction.toString(),
    };
  }

  PredictionStartWhenDirection.fromJson(Map<String, dynamic> json)
      : device = PredictionDevices.fromString(json['device']),
        channel = json['channel'],
        direction = PredictionDirections.fromString(json['direction']);
}

class PredictionEvent {
  PredictionEvent({
    required this.name,
    required this.previousEventName,
    required this.startWhen,
    required this.duration,
  });

  final String name;
  final String previousEventName;
  final List<PredictionStartWhen> startWhen;
  final Duration duration;

  Map<String, dynamic> serialize() {
    return {
      'name': name,
      'previous': previousEventName,
      'start_when': startWhen.map((e) => e.serialize()).toList(),
    };
  }

  PredictionEvent.fromSerialized(Map<String, dynamic> json, this.duration)
      : name = json['name'],
        previousEventName = json['previous'],
        startWhen = (json['start_when'] as List<dynamic>)
            .map((e) => PredictionStartWhen.factory(e))
            .toList();
}

enum PredictionAnalyzers {
  cyclicFromAnalogs;

  @override
  String toString() {
    switch (this) {
      case cyclicFromAnalogs:
        return 'cyclic_from_analogs';
    }
  }

  static PredictionAnalyzers fromString(String s) => PredictionAnalyzers.values
          .firstWhere((e) => e.toString() == s, orElse: () {
        throw Exception('Unknown analyzer: $s');
      });
}

enum PredictionDevices {
  delsysAnalogDataCollector,
  delsysEmgDataCollector;

  @override
  String toString() {
    switch (this) {
      case delsysAnalogDataCollector:
        return 'DelsysAnalogDataCollector';
      case delsysEmgDataCollector:
        return 'DelsysEmgDataCollector';
    }
  }

  static PredictionDevices fromString(String s) =>
      PredictionDevices.values.firstWhere((e) => e.toString() == s, orElse: () {
        throw Exception('Unknown device: $s');
      });
}

class PredictionModel {
  PredictionModel({
    required this.name,
    required this.analyzer,
    required this.timeReferenceDevice,
    required this.learningRate,
    required this.events,
  });

  final String name;
  final PredictionAnalyzers analyzer;
  final PredictionDevices timeReferenceDevice;
  final double learningRate;
  final List<PredictionEvent> events;

  Map<String, dynamic> serialize() => {
        'name': name,
        'analyzer_type': analyzer.toString(),
        'time_reference_device': timeReferenceDevice.toString(),
        'learning_rate': learningRate,
        'initial_phase_durations':
            events.map((e) => e.duration.inMilliseconds).toList(),
        'events': events.map((e) => e.serialize()).toList(),
      };

  PredictionModel.fromSerialized(Map<String, dynamic> json)
      : name = json['name'],
        analyzer = PredictionAnalyzers.fromString(json['analyzer_type']),
        timeReferenceDevice =
            PredictionDevices.fromString(json['time_reference_device']),
        learningRate = json['learning_rate'],
        events = (json['events'] as List<dynamic>)
            .asMap()
            .keys
            .map((index) => PredictionEvent.fromSerialized(
                json['events'][index],
                Duration(milliseconds: json['initial_phase_durations'][index])))
            .toList();
}

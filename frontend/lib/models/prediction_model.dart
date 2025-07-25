import 'package:frontend/utils/collections.dart';

enum PredictionStartWhenTypes {
  threshold,
  direction;

  @override
  String toString() {
    switch (this) {
      case threshold:
        return 'threshold';
      case direction:
        return 'direction';
    }
  }

  static PredictionStartWhenTypes fromString(String s) =>
      PredictionStartWhenTypes.values.firstWhere((e) => e.toString() == s,
          orElse: () {
        throw Exception('Unknown event type: $s');
      });
}

abstract class PredictionStartWhen {
  Map<String, dynamic> serialize();

  PredictionStartWhen copyWith();

  PredictionStartWhenTypes get type;

  static PredictionStartWhen factory(Map<String, dynamic> json) {
    final type = PredictionStartWhenTypes.fromString(json['type']);
    switch (type) {
      case PredictionStartWhenTypes.threshold:
        return PredictionStartWhenThreshold.fromJson(json);
      case PredictionStartWhenTypes.direction:
        return PredictionStartWhenDirection.fromJson(json);
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

  PredictionStartWhenThreshold.empty()
      : this(
            device: PredictionDevices.delsysEmgDataCollector,
            channel: 0,
            comparator: PredictionComparators.greaterThanOrEqual,
            value: 0.0);

  @override
  PredictionStartWhenTypes get type => PredictionStartWhenTypes.threshold;

  @override
  Map<String, dynamic> serialize() {
    return {
      'type': type.toString(),
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

  @override
  PredictionStartWhenThreshold copyWith({
    PredictionDevices? device,
    int? channel,
    PredictionComparators? comparator,
    double? value,
  }) {
    return PredictionStartWhenThreshold(
      device: device ?? this.device,
      channel: channel ?? this.channel,
      comparator: comparator ?? this.comparator,
      value: value ?? this.value,
    );
  }
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
  PredictionStartWhenTypes get type => PredictionStartWhenTypes.direction;

  @override
  Map<String, dynamic> serialize() {
    return {
      'type': type.toString(),
      'device': device.toString(),
      'channel': channel,
      'direction': direction.toString(),
    };
  }

  PredictionStartWhenDirection.empty()
      : this(
            device: PredictionDevices.delsysEmgDataCollector,
            channel: 0,
            direction: PredictionDirections.positive);

  PredictionStartWhenDirection.fromJson(Map<String, dynamic> json)
      : device = PredictionDevices.fromString(json['device']),
        channel = json['channel'],
        direction = PredictionDirections.fromString(json['direction']);

  @override
  PredictionStartWhenDirection copyWith({
    PredictionDevices? device,
    int? channel,
    PredictionDirections? direction,
  }) {
    return PredictionStartWhenDirection(
      device: device ?? this.device,
      channel: channel ?? this.channel,
      direction: direction ?? this.direction,
    );
  }
}

class PredictionEvent {
  PredictionEvent({
    required this.name,
    required this.previousEventName,
    required List<PredictionStartWhen> startWhen,
    required this.duration,
  }) : _startWhen = startWhen;

  final String name;
  final String previousEventName;
  final List<PredictionStartWhen> _startWhen;
  List<PredictionStartWhen> get startWhen => List.unmodifiable(_startWhen);
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
        _startWhen = (json['start_when'] as List<dynamic>)
            .map((e) => PredictionStartWhen.factory(e))
            .toList();

  PredictionEvent.empty({required this.name})
      : duration = const Duration(milliseconds: 0),
        previousEventName = name,
        _startWhen = [];

  PredictionEvent copyWith({
    String? name,
    String? previousEventName,
    List<PredictionStartWhen>? startWhen,
    Duration? duration,
  }) {
    return PredictionEvent(
      name: name ?? this.name,
      previousEventName: previousEventName ?? this.previousEventName,
      startWhen: startWhen ?? this.startWhen,
      duration: duration ?? this.duration,
    );
  }
}

enum PredictionAnalyzers {
  cyclicTimedEvents;

  @override
  String toString() {
    switch (this) {
      case cyclicTimedEvents:
        return 'Cyclic timed events';
    }
  }

  static PredictionAnalyzers fromJsonString(String s) =>
      PredictionAnalyzers.values.firstWhere((e) => e.jsonString() == s,
          orElse: () {
        throw Exception('Unknown analyzer: $s');
      });

  String jsonString() {
    switch (this) {
      case cyclicTimedEvents:
        return 'cyclic_timed_events';
    }
  }
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
    required List<PredictionEvent> events,
  }) : _events = events;

  final String name;
  final PredictionAnalyzers analyzer;
  final PredictionDevices timeReferenceDevice;
  final double learningRate;
  final List<PredictionEvent> _events;
  List<PredictionEvent> get events => List.unmodifiable(_events);

  Map<String, dynamic> serialize() => {
        'name': name,
        'analyzer_type': analyzer.jsonString(),
        'time_reference_device': timeReferenceDevice.toString(),
        'learning_rate': learningRate,
        'initial_phase_durations':
            events.map((e) => e.duration.inMilliseconds).toList(),
        'events': events.map((e) => e.serialize()).toList(),
      };

  PredictionModel.fromSerialized(Map<String, dynamic> json)
      : name = json['name'],
        analyzer = PredictionAnalyzers.fromJsonString(json['analyzer_type']),
        timeReferenceDevice =
            PredictionDevices.fromString(json['time_reference_device']),
        learningRate = json['learning_rate'],
        _events = (json['events'] as List<dynamic>)
            .asMap()
            .keys
            .map((index) => PredictionEvent.fromSerialized(
                json['events'][index],
                Duration(milliseconds: json['initial_phase_durations'][index])))
            .toList();

  PredictionModel.empty({required this.name})
      : analyzer = PredictionAnalyzers.cyclicTimedEvents,
        timeReferenceDevice = PredictionDevices.delsysEmgDataCollector,
        learningRate = 0.0,
        _events = [];

  PredictionModel copyWith({
    String? name,
    PredictionAnalyzers? analyzer,
    PredictionDevices? timeReferenceDevice,
    double? learningRate,
    List<PredictionEvent>? events,
  }) {
    return PredictionModel(
      name: name ?? this.name,
      analyzer: analyzer ?? this.analyzer,
      timeReferenceDevice: timeReferenceDevice ?? this.timeReferenceDevice,
      learningRate: learningRate ?? this.learningRate,
      events: events ?? this.events,
    );
  }

  @override
  bool operator ==(other) {
    if (identical(this, other)) return true;
    if (other is! PredictionModel) return false;

    return areMapsEqual(serialize(), other.serialize());
  }

  @override
  int get hashCode {
    return name.hashCode ^
        analyzer.hashCode ^
        timeReferenceDevice.hashCode ^
        learningRate.hashCode ^
        events.hashCode;
  }
}

class ScheduledStimulation {
  final String name;

  ScheduledStimulation.fromJson(Map<String, dynamic> json)
      : name = json['name'];

  @override
  String toString() => name;
}

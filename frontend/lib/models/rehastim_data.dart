class Channel {
  final int index;
  final double amplitude;

  Channel({
    required this.index,
    required this.amplitude,
  });
}

class Stimulation {
  final double t;
  final int duration;
  final List<Channel> channels;

  Stimulation({
    required this.t,
    required this.duration,
    required this.channels,
  });
}

class RehastimData {
  final double t0;
  final List<Stimulation> data;

  RehastimData({
    required this.t0,
    required this.data,
  });

  factory RehastimData.fromJson(Map<String, dynamic> json) {
    return RehastimData(
      t0: json['t0'],
      data: (json['data'] as List).map((stimulation) {
        return Stimulation(
          t: stimulation[0],
          duration: stimulation[1],
          channels: (stimulation[2] as List).map((channel) {
            return Channel(
              index: channel['channel_index'] as int,
              amplitude: channel['amplitude']!.toDouble(),
            );
          }).toList(),
        );
      }).toList(),
    );
  }
}

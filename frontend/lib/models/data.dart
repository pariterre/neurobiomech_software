import 'package:frontend/models/nidaq_data.dart';
import 'package:frontend/models/rehastim_data.dart';

class Data {
  final double t0;
  final NiDaqData nidaq;
  final RehastimData rehastim;

  int _previousLastIndex = -1;
  (int, int)? get lastBlockAddedIndex =>
      _previousLastIndex < 0 ? null : (_previousLastIndex, length);

  int get length => nidaq.t.length;

  void clear() {
    nidaq.clear();
    rehastim.clear();
    _previousLastIndex = -1;
  }

  Data({
    required this.t0,
    required int nbNidaqChannels,
    required int nbRehastimChannels,
  })  : nidaq = NiDaqData(nbChannels: nbNidaqChannels, t0: t0),
        rehastim = RehastimData(nbChannels: nbRehastimChannels, t0: t0);

  void appendFromJson(Map<String, dynamic> json) {
    _previousLastIndex = length;

    nidaq.appendFromJson(json['nidaq']);
    rehastim.appendFromJson(json['rehastim']);
  }
}

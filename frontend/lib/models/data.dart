import 'package:frontend/models/nidaq_data.dart';
import 'package:frontend/models/rehastim_data.dart';

class Data {
  final double t0;
  final double nidaqT0Offset;
  final NiDaqData nidaq;
  final RehastimData rehastim;

  int _previousLastIndex = -1;
  (int, int) get lastBlockAddedIndex => (_previousLastIndex, length);

  int get length => nidaq.t.length;

  Data({
    required this.t0,
    required this.nidaqT0Offset,
    required int nbNidaqChannels,
    required int nbRehastimChannels,
  })  : nidaq = NiDaqData(
          nbChannels: nbNidaqChannels,
          t0: t0,
          t0Offset: nidaqT0Offset,
        ),
        rehastim = RehastimData(nbChannels: nbRehastimChannels, t0: t0);

  void appendFromJson(Map<String, dynamic> json) {
    _previousLastIndex = length;
    nidaq.appendFromJson(json['nidaq']);
    rehastim.appendFromJson(json['rehastim']);
  }
}

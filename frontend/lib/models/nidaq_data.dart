class NiDaqData {
  final double t0;
  final List<double> t = [];
  final List<List<double>> data;

  void clear() {
    t.clear();
    for (var channel in data) {
      channel.clear();
    }
  }

  int get length => t.length;
  bool get isEmpty => t.isEmpty;
  bool get isNotEmpty => t.isNotEmpty;

  NiDaqData({
    required nbChannels,
    required this.t0,
  }) : data = List.generate(nbChannels, (_) => <double>[]);

  appendFromJson(Map<String, dynamic> json) {
    final dataTp = (json['data'] as List<dynamic>);
    for (int block = 0; block < dataTp.length; block++) {
      final dataBlock = dataTp[block] as List<dynamic>;
      for (int channel = 0; channel < dataBlock.length; channel++) {
        data[channel].addAll((dataTp[block][channel] as List).map(
          (e) => e as double,
        ));
      }
    }
    //print(this.t0);
    t.addAll((json['t'] as List<dynamic>).expand((e) => (e as List).map(
          (f) => (f as double),
        )));
  }
}

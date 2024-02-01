class NiDaqData {
  final double t0;
  final double t0Offset;
  final List<double> t = [];
  final List<List<double>> data;

  NiDaqData({
    required nbChannels,
    required this.t0,
    required this.t0Offset,
  }) : data = List.generate(nbChannels, (_) => <double>[]);

  appendFromJson(Map<String, dynamic> json) {
    t.addAll((json['t'] as List<dynamic>).expand((e) => (e as List).map(
          (f) => f as double,
        )));

    final dataTp = (json['data'] as List<dynamic>);
    for (int block = 0; block < dataTp.length; block++) {
      final dataBlock = dataTp[block] as List<dynamic>;
      for (int channel = 0; channel < dataBlock.length; channel++) {
        data[channel].addAll((dataTp[block][channel] as List).map(
          (e) => e as double,
        ));
      }
    }
  }
}

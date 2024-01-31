class NiDaqData {
  final double t0;
  final double t0Offset;
  final List<double> t;
  final List<double> data;

  NiDaqData({
    required this.t0,
    required this.t0Offset,
    required this.t,
    required this.data,
  });

  factory NiDaqData.fromJson(Map<String, dynamic> json) {
    return NiDaqData(
      t0: json['t0'],
      t0Offset: json['t0Offset'],
      t: json['t'].cast<double>(),
      data: json['data'].cast<double>(),
    );
  }
}

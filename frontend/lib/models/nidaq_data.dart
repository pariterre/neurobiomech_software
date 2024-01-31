class NiDaqData {
  final double t0;
  final double t0Offset;
  final List<List<double>> t;
  final List<List<List<double>>> data;

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
      t: (json['t'] as List).cast<List<double>>(),
      data: (json['data'] as List).cast<List<List<double>>>(),
    );
  }
}

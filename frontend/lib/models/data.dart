import 'package:frontend/models/nidaq_data.dart';
import 'package:frontend/models/rehastim_data.dart';

class Data {
  final NiDaqData nidaq;
  final RehastimData rehastim;

  Data({
    required this.nidaq,
    required this.rehastim,
  });

  factory Data.fromJson(Map<String, dynamic> json) {
    return Data(
      nidaq: NiDaqData.fromJson(json['nidaq']),
      rehastim: RehastimData.fromJson(json['rehastim']),
    );
  }
}

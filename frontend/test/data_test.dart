// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/time_series_data.dart';

void main() {
  test('Data', () {
    final data = Data(
        dataGenericType: DataGenericTypes.analogs,
        initialTime: DateTime(1),
        analogChannelCount: 2,
        emgChannelCount: 2,
        isFromLiveData: false);

    data.delsysAnalog.appendFromJson({
      'starting_time': 0,
      'data': [
        [
          100,
          [1.0, 1.1]
        ],
        [
          200,
          [1.4, 1.5]
        ],
        [
          300,
          [1.8, 1.9]
        ]
      ]
    });

    data.delsysEmg.appendFromJson({
      'starting_time': 0,
      'data': [
        [
          100,
          [1.0, 1.1]
        ],
        [
          200,
          [1.4, 1.5]
        ],
        [
          300,
          [1.8, 1.9]
        ]
      ]
    });

    expect(data.initialTime, DateTime(1));
    expect(data.delsysAnalog.length, 3);
    expect(data.delsysEmg.length, 3);

    data.clear();
    expect(data.delsysAnalog.length, 0);
    expect(data.delsysEmg.length, 0);
  });

  test('TimeSeriesData', () {
    final data = TimeSeriesData(
        initialTime: DateTime(1000), channelCount: 2, isFromLiveData: false);
    data.appendFromJson({
      'starting_time': 10,
      'data': [
        [
          0,
          [1.0, 1.1]
        ],
        [
          25,
          [1.4, 1.5]
        ],
        [
          50,
          [1.8, 1.9]
        ]
      ]
    });
    data.appendFromJson({
      'data': [
        [
          75,
          [2.2, 2.3]
        ],
        [
          100,
          [2.6, 2.7]
        ],
        [
          125,
          [3.0, 3.1]
        ]
      ]
    });

    expect(
        data.time,
        Iterable.generate(
            6, (i) => ((i.toDouble() * 0.025) * 1000).toInt() / 1000).toList());
    expect(
        data.getData()[0],
        Iterable.generate(
                6, (i) => double.parse((1.0 + i * 0.4).toStringAsFixed(2)))
            .toList());
  });
}

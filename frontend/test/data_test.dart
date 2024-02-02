// This is a basic Flutter widget test.
//
// To perform an interaction with a widget in your test, use the WidgetTester
// utility in the flutter_test package. For example, you can send tap and scroll
// gestures. You can also use WidgetTester to find child widgets in the widget
// tree, read text, and verify that the values of widget properties are correct.

import 'package:flutter_test/flutter_test.dart';
import 'package:frontend/models/nidaq_data.dart';
import 'package:frontend/models/rehastim_data.dart';

void main() {
  test('NidaqData', () {
    final data = NiDaqData(nbChannels: 2, t0: 1000.0, t0Offset: 0.01);
    data.appendFromJson({
      "t": [
        [0.01, 0.26, 0.51, 0.76],
        [1.01, 1.26, 1.51, 1.76],
        [2.01, 2.26, 2.51, 2.76]
      ],
      "data": [
        [
          [1.0, 1.1, 1.2, 1.3],
          [-1.0, -1.1, -1.2, -1.3]
        ],
        [
          [1.4, 1.5, 1.6, 1.7],
          [-1.4, -1.5, -1.6, -1.7]
        ],
        [
          [1.8, 1.9, 2.0, 2.1],
          [-1.8, -1.9, -2.0, -2.1]
        ]
      ]
    });
    data.appendFromJson({
      "t": [
        [3.01, 3.26, 3.51, 3.76],
        [4.01, 4.26, 4.51, 4.76],
        [5.01, 5.26, 5.51, 5.76]
      ],
      "data": [
        [
          [2.2, 2.3, 2.4, 2.5],
          [-2.2, -2.3, -2.4, -2.5]
        ],
        [
          [2.6, 2.7, 2.8, 2.9],
          [-2.6, -2.7, -2.8, -2.9]
        ],
        [
          [3.0, 3.1, 3.2, 3.3],
          [-3.0, -3.1, -3.2, -3.3]
        ]
      ]
    });

    expect(data.t0, 1000.0);
    expect(data.t0Offset, 0.01);
    expect(data.t, Iterable.generate(24, (i) => 0.01 + i * 0.25).toList());
    expect(
        data.data[0],
        Iterable.generate(
                24, (i) => double.parse((1.0 + i * 0.1).toStringAsFixed(2)))
            .toList());
  });

  test('RehastimData', () {
    final data = RehastimData(nbChannels: 2, t0: 0.1);
    data.appendFromJson({
      "t0": 0.1,
      "data": [
        [
          0.2,
          2.0,
          [
            {"channel_index": 1, "amplitude": 2},
            {"channel_index": 2, "amplitude": 4}
          ]
        ],
        [
          1.0,
          3.0,
          [
            {"channel_index": 1, "amplitude": 2},
            {"channel_index": 2, "amplitude": 4}
          ]
        ]
      ]
    });

    expect(data.t0, 0.1);
    expect(data.data.length, 2);
    expect(data.data[0].t, 0.2);
    expect(data.data[0].duration, 2);
    expect(data.data[0].channels.length, 2);
    expect(data.data[0].channels[0].index, 1);
    expect(data.data[0].channels[0].amplitude, 2);
    expect(data.data[0].channels[1].index, 2);
    expect(data.data[0].channels[1].amplitude, 4);
    expect(data.data[1].t, 1.0);
    expect(data.data[1].duration, 3);
    expect(data.data[1].channels.length, 2);
    expect(data.data[1].channels[0].index, 1);
    expect(data.data[1].channels[0].amplitude, 2);
    expect(data.data[1].channels[1].index, 2);
    expect(data.data[1].channels[1].amplitude, 4);
  });
}

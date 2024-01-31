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
    final data = NiDaqData.fromJson({
      "t0": 1000.0,
      "t0Offset": 0.01,
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
          [2.0, 2.1, 2.2, 2.3],
          [-2.0, -2.1, -2.2, -2.3]
        ],
        [
          [3.0, 3.1, 3.2, 3.3],
          [-3.0, -3.1, -3.2, -3.3]
        ]
      ]
    });

    expect(data.t0, 1000.0);
    expect(data.t0Offset, 0.01);
    expect(data.t, [
      [0.01, 0.26, 0.51, 0.76],
      [1.01, 1.26, 1.51, 1.76],
      [2.01, 2.26, 2.51, 2.76]
    ]);
    expect(data.data, [
      [
        [1.0, 1.1, 1.2, 1.3],
        [-1.0, -1.1, -1.2, -1.3]
      ],
      [
        [2.0, 2.1, 2.2, 2.3],
        [-2.0, -2.1, -2.2, -2.3]
      ],
      [
        [3.0, 3.1, 3.2, 3.3],
        [-3.0, -3.1, -3.2, -3.3]
      ]
    ]);
  });

  test('RehastimData', () {
    final data = RehastimData.fromJson({
      "t0": 0.1,
      "data": [
        [
          0.2,
          2,
          [
            {"channel_index": 1, "amplitude": 2},
            {"channel_index": 2, "amplitude": 4}
          ]
        ],
        [
          1.0,
          3,
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

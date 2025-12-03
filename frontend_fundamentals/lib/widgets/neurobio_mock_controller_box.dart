import 'package:flutter/material.dart';
import 'package:frontend_fundamentals/managers/neurobio_client.dart';

class NeurobioMockControllerBox extends StatefulWidget {
  const NeurobioMockControllerBox({
    super.key,
    required this.child,
    this.initialPosition = const Offset(0, 0),
  });

  final Widget child;
  final Offset initialPosition;

  @override
  State<NeurobioMockControllerBox> createState() =>
      _NeurobioMockControllerBoxState();
}

class _NeurobioMockControllerBoxState extends State<NeurobioMockControllerBox> {
  final _neurobioClient = NeurobioClient.instance;
  late final bool _isMock = _neurobioClient is NeurobioClientMock;
  late final _controller = _isMock
      ? (_neurobioClient as NeurobioClientMock).controller
      : null;

  var _boxPosition = Offset(50, 50);
  var _dragStartOffset = Offset.zero;

  @override
  void initState() {
    super.initState();

    if (_isMock) {
      _neurobioClient.onDeviceConnectionChanged.addListener(_refresh);
    }
  }

  @override
  void dispose() {
    if (_isMock) {
      _neurobioClient.onDeviceConnectionChanged.removeListener(_refresh);
    }
    super.dispose();
  }

  void _refresh() {
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    if (!_isMock) return widget.child;
    final emgCount = _controller!.liveAnalogsData.delsysEmg.channelCount;

    return MaterialApp(
      home: Scaffold(
        body: Stack(
          children: [
            widget.child,
            Positioned(
              left: _boxPosition.dx,
              top: _boxPosition.dy,
              child: GestureDetector(
                onPanStart: (details) =>
                    _dragStartOffset = details.globalPosition - _boxPosition,
                onPanUpdate: (details) {
                  setState(() {
                    _boxPosition = details.globalPosition - _dragStartOffset;
                  });
                },
                child: Container(
                  decoration: BoxDecoration(
                    color: Colors.amber,
                    borderRadius: BorderRadius.circular(10),
                  ),
                  child: ConstrainedBox(
                    constraints: BoxConstraints(maxHeight: 400),
                    child: SingleChildScrollView(
                      child: Padding(
                        padding: const EdgeInsets.all(8.0),
                        child: Column(
                          children: [
                            Padding(
                              padding: const EdgeInsets.all(12.0),
                              child: Center(
                                child: Text(
                                  'Channels controller',
                                  style: TextStyle(
                                    fontWeight: FontWeight.bold,
                                    fontSize: 16,
                                  ),
                                ),
                              ),
                            ),
                            if (_neurobioClient.isConnectedToDelsysEmg)
                              Column(
                                children: [
                                  Text('EMG Channels'),
                                  for (int i = 0; i < emgCount; i++)
                                    _channelSliderBuilder(i),
                                ],
                              ),
                          ],
                        ),
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  Widget _channelSliderBuilder(int channelIndex) {
    return Row(
      children: [
        Text('Channel $channelIndex'),
        Slider(
          value: _controller!.getEmgChannelValue(channel: channelIndex),
          min: -10.0,
          max: 10.0,
          divisions: 200,
          label: _controller
              .getEmgChannelValue(channel: channelIndex)
              .toStringAsFixed(2),
          onChanged: (newValue) {
            setState(() {
              _controller.setEmgChannelValue(
                channel: channelIndex,
                value: newValue,
              );
            });
          },
        ),
      ],
    );
  }
}

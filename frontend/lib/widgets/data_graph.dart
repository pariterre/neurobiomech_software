import 'dart:math';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/time_series_data.dart';

enum DataGraphType { emg, analog, predictions }

class DataGraphController {
  Data _data;
  final DataGraphType graphType;

  Function()? _updateCallback;

  set data(Data data) {
    _data = data;
    if (_updateCallback != null) _updateCallback!();
  }

  DataGraphController({required Data data, required this.graphType})
      : _data = data;
}

class DataGraph extends StatefulWidget {
  const DataGraph(
      {super.key, required this.controller, this.combineGraphs = false});

  final bool combineGraphs;
  final DataGraphController controller;

  @override
  State<DataGraph> createState() => _DataGraphState();
}

class _DataGraphState extends State<DataGraph> {
  @override
  void initState() {
    super.initState();
    widget.controller._updateCallback = _redraw;
  }

  @override
  void dispose() {
    widget.controller._updateCallback = null;
    super.dispose();
  }

  DateTime _lastRefresh = DateTime.now();
  void _redraw() {
    if (DateTime.now().difference(_lastRefresh).inMilliseconds < 100) return;
    _lastRefresh = DateTime.now();
    if (!mounted) return;
    setState(() {});
  }

  Iterable<LineChartBarData?> _dataToLineBarsData({int? channel}) {
    final TimeSeriesData timeSeries = _timeSeries;

    final time = timeSeries.time;
    return timeSeries.getData().asMap().entries.map((e) =>
        (channel != null && channel == e.key && _showChannels[e.key]) ||
                (channel == null && _showChannels[e.key])
            ? LineChartBarData(
                color: Colors.black,
                spots: e.value
                    .asMap()
                    .entries
                    .map((entry) => FlSpot(time[entry.key] / 1000.0,
                        entry.value * _multipliers[e.key]))
                    .toList(),
                isCurved: false,
                isStrokeCapRound: false,
                barWidth: 1,
                dotData: const FlDotData(show: false),
              )
            : null);
  }

  TimeSeriesData get _timeSeries {
    switch (widget.controller.graphType) {
      case DataGraphType.emg:
        return widget.controller._data.delsysEmg;
      case DataGraphType.analog:
        return widget.controller._data.delsysAnalog;
      case DataGraphType.predictions:
        return widget.controller._data.predictions;
    }
  }

  int get _channelCount => _timeSeries.channelCount;

  bool _combineChannels = true;
  void _onChanged(bool combineChannels) {
    setState(() {
      _combineChannels = combineChannels;
    });
  }

  late final List<bool> _showChannels =
      List.generate(_channelCount, (_) => true);
  void _onChannelSelected(int channel, bool newValue) {
    setState(() {
      _showChannels[channel] = newValue;
    });
  }

  late final List<bool> _computeRms = List.generate(
      _channelCount,
      (i) => widget.controller.graphType == DataGraphType.emg
          ? widget.controller._data.delsysEmg.getApplySlidingRms(channel: i)
          : false);
  void _onComputeRmsSelected(int channel, bool newValue) {
    if (widget.controller.graphType != DataGraphType.emg) return;

    widget.controller._data.delsysEmg
        .setApplySlidingRms(newValue, channel: channel);
    setState(() {
      _computeRms[channel] = newValue;
    });
  }

  late final List<double> _multipliers = List.generate(
      _channelCount,
      (_) =>
          widget.controller.graphType == DataGraphType.predictions ? 100 : 1);

  void _onMultiplierChanged(int channel, double newValue) {
    setState(() {
      _multipliers[channel] = newValue;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        Column(
          children: [
            const SizedBox(height: 65),
            SizedBox(
              height: 400,
              width: double.infinity,
              child: Padding(
                padding: const EdgeInsets.only(
                  left: 12,
                  bottom: 12,
                  right: 20,
                  top: 20,
                ),
                child: LayoutBuilder(
                  builder: (context, constraints) {
                    return _combineChannels
                        ? _buildLineChart(constraints)
                        : ListView.builder(
                            itemCount: _channelCount,
                            itemBuilder: (context, index) =>
                                _showChannels[index]
                                    ? Padding(
                                        padding:
                                            const EdgeInsets.only(bottom: 10.0),
                                        child: SizedBox(
                                            height: 90,
                                            child: _buildLineChart(constraints,
                                                channel: index)),
                                      )
                                    : const SizedBox(),
                          );
                  },
                ),
              ),
            ),
          ],
        ),
        Stack(
          children: [
            Align(
              alignment: Alignment.center,
              child: Row(
                mainAxisSize: MainAxisSize.min,
                children: [
                  _RadioCombineChannels(
                    combineChannels: _combineChannels,
                    onChanged: _onChanged,
                    graphType: widget.controller.graphType,
                  ),
                  if (widget.controller.graphType == DataGraphType.emg &&
                      widget.controller._data.delsysEmg.isFromLiveData)
                    SizedBox(
                      width: 150,
                      child: TextFormField(
                        decoration: const InputDecoration(
                          labelText: 'Sliding window size',
                        ),
                        initialValue: widget.controller._data.delsysEmg
                            .getSlidingRmsWindow(channel: 0)
                            .toString(),
                        inputFormatters: [
                          FilteringTextInputFormatter.digitsOnly
                        ],
                        onChanged: (value) {
                          final valueAsInt = int.tryParse(value);
                          if (valueAsInt == null) return;
                          for (var i = 0; i < _channelCount; i++) {
                            widget.controller._data.delsysEmg
                                .setSlidingRmsWindow(valueAsInt, channel: i);
                          }
                        },
                      ),
                    )
                ],
              ),
            ),
            Align(
              alignment: Alignment.centerRight,
              child: Padding(
                  padding: const EdgeInsets.only(top: 14.0, right: 8.0),
                  child: _ChannelOptionsPopup(
                    onChannelSelected: _onChannelSelected,
                    showChannels: _showChannels,
                    onComputeRmsSelected:
                        widget.controller.graphType == DataGraphType.emg &&
                                widget.controller._data.delsysEmg.isFromLiveData
                            ? _onComputeRmsSelected
                            : null,
                    computeRms: _computeRms,
                    onMultiplierChanged: _onMultiplierChanged,
                    multipliers: _multipliers,
                    graphType: widget.controller.graphType,
                  )),
            ),
          ],
        ),
      ],
    );
  }

  Widget _buildLineChart(BoxConstraints constraints, {int? channel}) {
    var data = _dataToLineBarsData(channel: channel).where((e) => e != null);

    return AspectRatio(
      aspectRatio: 1,
      child: LineChart(
        LineChartData(
          lineTouchData: _lineTouchData(),
          lineBarsData: data.map((e) => e!).toList(),
          titlesData: _titlesData(constraints),
          gridData: const FlGridData(
            show: true,
            drawHorizontalLine: true,
            drawVerticalLine: true,
            horizontalInterval: null,
            verticalInterval: null,
          ),
          borderData: FlBorderData(show: true),
        ),
        duration: Duration.zero,
      ),
    );
  }
}

LineTouchData _lineTouchData() {
  return LineTouchData(
    touchTooltipData: LineTouchTooltipData(
      maxContentWidth: 100,
      getTooltipColor: (touchedSpot) => Colors.grey[800]!,
      getTooltipItems: (touchedSpots) {
        return touchedSpots
            .map((LineBarSpot touchedSpot) => LineTooltipItem(
                  '${touchedSpot.x}, ${touchedSpot.y.toStringAsFixed(4)}',
                  const TextStyle(
                    color: Colors.white,
                    fontSize: 12,
                  ),
                ))
            .toList();
      },
    ),
    handleBuiltInTouches: false, // Change this to add the values
    getTouchLineStart: (data, index) => 0,
  );
}

FlTitlesData _titlesData(BoxConstraints constraints,
        {TextStyle style = const TextStyle(fontWeight: FontWeight.bold)}) =>
    FlTitlesData(
      leftTitles: AxisTitles(
        sideTitles: SideTitles(
          showTitles: true,
          getTitlesWidget: (value, meta) => value % 1 != 0
              ? Container()
              : SideTitleWidget(
                  axisSide: meta.axisSide,
                  space: 16,
                  child: Text(meta.formattedValue,
                      style: style.copyWith(
                          fontSize: min(18, 18 * constraints.maxWidth / 300))),
                ),
          reservedSize: 56,
        ),
        drawBelowEverything: true,
      ),
      rightTitles: const AxisTitles(
        sideTitles: SideTitles(showTitles: false),
      ),
      bottomTitles: AxisTitles(
        sideTitles: SideTitles(
          showTitles: true,
          getTitlesWidget: (value, meta) => value % 1 != 0
              ? Container()
              : SideTitleWidget(
                  axisSide: meta.axisSide,
                  space: 16,
                  child: Text(meta.formattedValue,
                      style: style.copyWith(
                          fontSize: min(18, 18 * constraints.maxWidth / 300))),
                ),
          reservedSize: 36,
          interval: 1,
        ),
        drawBelowEverything: true,
      ),
      topTitles: const AxisTitles(
        sideTitles: SideTitles(showTitles: false),
      ),
    );

class _RadioCombineChannels extends StatelessWidget {
  const _RadioCombineChannels({
    required this.combineChannels,
    required this.onChanged,
    required this.graphType,
  });

  final bool combineChannels;
  final Function(bool) onChanged;
  final DataGraphType graphType;

  @override
  Widget build(BuildContext context) {
    final channelsName =
        graphType == DataGraphType.predictions ? 'predictions' : 'channels';

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        SizedBox(
          width: 200,
          child: RadioListTile<bool>(
            title: Text('Combine $channelsName'),
            value: true,
            groupValue: combineChannels,
            onChanged: (value) => onChanged(value!),
          ),
        ),
        SizedBox(
          width: 200,
          child: RadioListTile<bool>(
            title: Text('Separate $channelsName'),
            value: false,
            groupValue: combineChannels,
            onChanged: (value) => onChanged(value!),
          ),
        ),
      ],
    );
  }
}

class _ChannelOptionsPopup extends StatefulWidget {
  const _ChannelOptionsPopup({
    required this.onChannelSelected,
    required this.showChannels,
    required this.onComputeRmsSelected,
    required this.computeRms,
    required this.onMultiplierChanged,
    required this.multipliers,
    required this.graphType,
  });

  final Function(int index, bool value) onChannelSelected;
  final List<bool> showChannels;

  final Function(int index, bool value)? onComputeRmsSelected;
  final List<bool>? computeRms;

  final Function(int index, double value)? onMultiplierChanged;
  final List<double>? multipliers;

  final DataGraphType graphType;

  @override
  State<_ChannelOptionsPopup> createState() => _ChannelOptionsPopupState();
}

class _ChannelOptionsPopupState extends State<_ChannelOptionsPopup> {
  bool _isExpanded = false;
  bool get _useRms =>
      widget.computeRms != null && widget.onComputeRmsSelected != null;

  bool get _useMultiplier => widget.multipliers != null;

  bool get _canShowSelectAll => widget.showChannels.any((element) => !element);
  bool get _canComputeRmsAll =>
      _useRms && widget.computeRms!.any((element) => !element);

  void _onExpand() {
    setState(() {
      _isExpanded = !_isExpanded;
    });
  }

  @override
  Widget build(BuildContext context) {
    var width = widget.graphType == DataGraphType.predictions ? 40.0 : 10.0;
    const indexWidth = 40.0;
    const showWidth = 40.0;
    const rmsWidth = 40.0;
    const multiplierWidth = 60.0;

    width += indexWidth;
    width += showWidth;
    if (_useRms) width += rmsWidth;
    if (_useMultiplier) width += multiplierWidth;

    return SizedBox(
      width: width,
      child: Container(
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(8),
          border: Border.all(color: Colors.black),
        ),
        child: Column(
          children: [
            GestureDetector(
              onTap: _onExpand,
              child: Container(
                width: double.infinity,
                color: Colors.grey,
                padding: const EdgeInsets.symmetric(vertical: 4.0),
                child: Center(
                  child: Row(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Text(widget.graphType == DataGraphType.predictions
                          ? 'Prédictions'
                          : 'Channels'),
                      const SizedBox(width: 12),
                      Icon(_isExpanded
                          ? Icons.arrow_drop_up
                          : Icons.arrow_drop_down),
                    ],
                  ),
                ),
              ),
            ),
            if (_isExpanded)
              Container(
                color: Colors.grey[100],
                width: double.infinity,
                child: Row(
                  mainAxisSize: MainAxisSize.min,
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    const SizedBox(
                        width: indexWidth, child: Center(child: Text('All'))),
                    SizedBox(
                      width: showWidth,
                      child: Column(
                        children: [
                          const Text('Show'),
                          Checkbox(
                            onChanged: (_) {
                              final value = _canShowSelectAll;
                              for (var i = 0;
                                  i < widget.showChannels.length;
                                  i++) {
                                widget.onChannelSelected(i, value);
                              }
                              setState(() {});
                            },
                            value: !_canShowSelectAll,
                          ),
                        ],
                      ),
                    ),
                    if (_useRms)
                      SizedBox(
                        width: rmsWidth,
                        child: Column(
                          children: [
                            const Text('RMS'),
                            Checkbox(
                              onChanged: (_) {
                                final value = _canComputeRmsAll;
                                for (var i = 0;
                                    i < widget.computeRms!.length;
                                    i++) {
                                  widget.onComputeRmsSelected!(i, value);
                                }
                                setState(() {});
                              },
                              value: !_canComputeRmsAll,
                            ),
                          ],
                        ),
                      ),
                    if (_useMultiplier)
                      const SizedBox(
                        width: multiplierWidth,
                        child: Center(
                          child: Text('X mult'),
                        ),
                      ),
                  ],
                ),
              ),
            if (_isExpanded)
              ...List.generate(
                widget.showChannels.length,
                (index) => _OptionsBuilder(
                  index: index,
                  indexWidth: indexWidth,
                  showWidth: showWidth,
                  showChannels: widget.showChannels,
                  onChannelSelected: (_) {
                    widget.onChannelSelected(
                        index, !widget.showChannels[index]);
                    setState(() {});
                  },
                  rmsWidth: rmsWidth,
                  computeRms: widget.computeRms,
                  onComputeRmsSelected: _useRms
                      ? (_) {
                          widget.onComputeRmsSelected!(
                              index, !widget.computeRms![index]);
                          setState(() {});
                        }
                      : null,
                  multiplierWidth: multiplierWidth,
                  multipliers: widget.multipliers,
                  onMultiplierChanged: _useMultiplier
                      ? (value) => setState(
                          () => widget.onMultiplierChanged!(index, value))
                      : null,
                ),
              ),
          ],
        ),
      ),
    );
  }
}

class _OptionsBuilder extends StatefulWidget {
  const _OptionsBuilder({
    required this.index,
    required this.indexWidth,
    required this.showWidth,
    required this.showChannels,
    required this.onChannelSelected,
    required this.rmsWidth,
    required this.computeRms,
    required this.onComputeRmsSelected,
    required this.multiplierWidth,
    required this.multipliers,
    required this.onMultiplierChanged,
  });

  final int index;
  final double indexWidth;

  final double showWidth;
  final List<bool> showChannels;
  final Function(bool? value) onChannelSelected;

  final double rmsWidth;
  final List<bool>? computeRms;
  final Function(bool? value)? onComputeRmsSelected;

  final double multiplierWidth;
  final List<double>? multipliers;
  final Function(double value)? onMultiplierChanged;

  @override
  State<_OptionsBuilder> createState() => _OptionsBuilderState();
}

class _OptionsBuilderState extends State<_OptionsBuilder> {
  late final _controller =
      TextEditingController(text: widget.multipliers?[widget.index].toString());
  final _focusNode = FocusNode();

  @override
  void dispose() {
    _controller.dispose();
    _focusNode.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      color: widget.index % 2 == 0 ? Colors.grey[200] : Colors.grey[100],
      width: double.infinity,
      child: Row(
        mainAxisSize: MainAxisSize.min,
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          SizedBox(
              width: widget.indexWidth,
              child: Center(child: Text((widget.index + 1).toString()))),
          SizedBox(
            width: widget.showWidth,
            child: Checkbox(
                onChanged: widget.onChannelSelected,
                value: widget.showChannels[widget.index]),
          ),
          if (widget.onComputeRmsSelected != null)
            SizedBox(
              width: widget.rmsWidth,
              child: Center(
                child: Checkbox(
                    onChanged: widget.onComputeRmsSelected,
                    value: widget.computeRms![widget.index]),
              ),
            ),
          if (widget.onMultiplierChanged != null)
            SizedBox(
              width: widget.multiplierWidth,
              child: TextField(
                controller: _controller,
                focusNode: _focusNode,
                onChanged: (value) {
                  final valueAsDouble = double.tryParse(value);
                  if (valueAsDouble == null) return;
                  widget.onMultiplierChanged!(valueAsDouble);
                },
                textAlign: TextAlign.center,
              ),
            ),
        ],
      ),
    );
  }
}

import 'dart:math';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:frontend/models/data.dart';
import 'package:frontend/models/time_series_data.dart';

enum DataGraphType { emg, analog }

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
    return timeSeries.data.asMap().entries.map((e) => (channel != null &&
                channel == e.key &&
                _selectedChannels[e.key]) ||
            (channel == null && _selectedChannels[e.key])
        ? LineChartBarData(
            color: Colors.black,
            spots: e.value
                .asMap()
                .entries
                .map((entry) => FlSpot(time[entry.key] / 1000.0, entry.value))
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
    }
  }

  int get _channelCount => _timeSeries.channelCount;

  bool _combineChannels = true;
  void _onChanged(bool combineChannels) {
    setState(() {
      _combineChannels = combineChannels;
    });
  }

  late final List<bool> _selectedChannels =
      List.generate(_channelCount, (_) => true);
  void _onChannelSelected(int channel, bool newValue) {
    setState(() {
      _selectedChannels[channel] = newValue;
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
                                _selectedChannels[index]
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
              child: _RadioCombineChannels(
                combineChannels: _combineChannels,
                onChanged: _onChanged,
              ),
            ),
            Align(
              alignment: Alignment.centerRight,
              child: Padding(
                padding: const EdgeInsets.only(top: 14.0, right: 8.0),
                child: _SelectChannelsPopup(
                    onChannelSelected: _onChannelSelected,
                    selectedChannels: _selectedChannels),
              ),
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
            horizontalInterval: 1.0,
            verticalInterval: 0.1,
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
        return touchedSpots.map((LineBarSpot touchedSpot) {
          const textStyle = TextStyle(
            color: Colors.white,
            fontSize: 12,
          );
          return LineTooltipItem(
            '${touchedSpot.x}, ${touchedSpot.y.toStringAsFixed(4)}',
            textStyle,
          );
        }).toList();
      },
    ),
    handleBuiltInTouches: true,
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
  const _RadioCombineChannels(
      {required this.combineChannels, required this.onChanged});

  final bool combineChannels;
  final Function(bool) onChanged;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        SizedBox(
          width: 200,
          child: RadioListTile<bool>(
            title: const Text('Combine channels'),
            value: true,
            groupValue: combineChannels,
            onChanged: (value) => onChanged(value!),
          ),
        ),
        SizedBox(
          width: 200,
          child: RadioListTile<bool>(
            title: const Text('Separate channels'),
            value: false,
            groupValue: combineChannels,
            onChanged: (value) => onChanged(value!),
          ),
        ),
      ],
    );
  }
}

class _SelectChannelsPopup extends StatefulWidget {
  const _SelectChannelsPopup(
      {required this.onChannelSelected, required this.selectedChannels});

  final Function(int index, bool value) onChannelSelected;
  final List<bool> selectedChannels;

  @override
  State<_SelectChannelsPopup> createState() => _SelectChannelsPopupState();
}

class _SelectChannelsPopupState extends State<_SelectChannelsPopup> {
  bool _isExpanded = false;

  bool get _canSelectAll => widget.selectedChannels.any((element) => !element);

  void _onExpand() {
    setState(() {
      _isExpanded = !_isExpanded;
    });
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 175,
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
                      const Text('Select channels'),
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
                child: CheckboxListTile(
                    onChanged: (_) {
                      final value = _canSelectAll;
                      for (var i = 0; i < widget.selectedChannels.length; i++) {
                        widget.onChannelSelected(i, value);
                      }
                      setState(() {});
                    },
                    value: false,
                    title: Text(_canSelectAll ? 'Select all' : 'Deselect all')),
              ),
            if (_isExpanded)
              ...List.generate(
                widget.selectedChannels.length,
                (index) => Container(
                  color: index % 2 == 0 ? Colors.grey[200] : Colors.grey[100],
                  child: CheckboxListTile(
                      onChanged: (_) {
                        widget.onChannelSelected(
                            index, !widget.selectedChannels[index]);
                        setState(() {});
                      },
                      value: widget.selectedChannels[index],
                      title: Text(index.toString())),
                ),
              ),
          ],
        ),
      ),
    );
  }
}

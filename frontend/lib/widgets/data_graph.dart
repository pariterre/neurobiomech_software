import 'dart:math';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:frontend/models/data.dart';

class DataGraph extends StatelessWidget {
  const DataGraph({super.key, required this.data});

  final Data data;

  List<LineChartBarData> _dataToLineBarsData() {
    final time = data.delsysEmg.t;
    return data.delsysEmg.data
        .asMap()
        .entries
        .map((channel) => LineChartBarData(
              color: Colors.black,
              spots: channel.value
                  .asMap()
                  .entries
                  .map((entry) => FlSpot(
                      time[entry.key], entry.value + (0.1 * channel.key)))
                  .toList(),
              isCurved: false,
              isStrokeCapRound: false,
              barWidth: 1,
              dotData: const FlDotData(show: false),
            ))
        .toList();
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      height: 400,
      width: double.infinity,
      child: Padding(
        padding: const EdgeInsets.only(
          left: 12,
          bottom: 12,
          right: 20,
          top: 20,
        ),
        child: AspectRatio(
          aspectRatio: 1,
          child: LayoutBuilder(
            builder: (context, constraints) {
              return LineChart(
                LineChartData(
                  lineTouchData: _lineTouchData(),
                  lineBarsData: _dataToLineBarsData(),
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
              );
            },
          ),
        ),
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

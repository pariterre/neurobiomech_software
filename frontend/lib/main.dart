import 'package:flutter/material.dart';
import 'package:frontend/screens/debug_screen.dart';

import 'package:logging/logging.dart';

void main() async {
  // Configure logging
  Logger.root.onRecord.listen((record) {
    if (record.level >= Level.INFO) {
      debugPrint('${record.level.name}: ${record.time}: ${record.message}');
    }
  });

  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Lokomat FES Server Interface',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      initialRoute: DebugScreen.route,
      routes: {
        DebugScreen.route: (context) => const DebugScreen(),
      },
    );
  }
}

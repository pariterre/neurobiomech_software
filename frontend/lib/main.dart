import 'package:flutter/material.dart';
import 'package:frontend/managers/database_manager.dart';
import 'package:frontend/managers/predictions_manager.dart';
import 'package:frontend/screens/main_screen.dart';
import 'package:logging/logging.dart';

void main() async {
  // Configure logging
  Logger.root.onRecord.listen((record) {
    if (record.level >= Level.INFO) {
      debugPrint('${record.level.name}: ${record.time}: ${record.message}');
    }
  });

  // Instantiate the DatabaseManager
  WidgetsFlutterBinding.ensureInitialized();
  DatabaseManager.instance;
  await PredictionsManager.instance.initialize();

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
      initialRoute: MainScreen.route,
      routes: {
        MainScreen.route: (context) => const MainScreen(),
      },
    );
  }
}

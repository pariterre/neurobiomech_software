import 'dart:io';

import 'package:shared_preferences/shared_preferences.dart';

class DatabaseManager {
  ///
  /// Create the singleton instance
  static final _instance = DatabaseManager._();
  static DatabaseManager get instance => _instance;
  DatabaseManager._() {
    _initializeDatabaseManager();
  }

  Future<void> _initializeDatabaseManager() async {
    final prefs = await SharedPreferences.getInstance();

    databaseFolder = prefs.getString('databaseFolder') ?? 'data';
    project = prefs.getString('project') ?? 'defaultProject';
    subject = prefs.getString('subject') ?? 'defaultSubject';
    lastTrial = prefs.getString('lastTrial') ?? 'Trial1';
  }

  ///
  /// The folder to save the data
  String _databaseFolder = '';
  String get databaseFolder => _databaseFolder;
  set databaseFolder(String value) {
    _databaseFolder = Directory(value).absolute.path;
    SharedPreferences.getInstance().then((prefs) {
      prefs.setString('databaseFolder', _databaseFolder);
    });
  }

  ///
  /// Current project
  String _project = '';
  String get project => _project;
  set project(String value) {
    _project = value;
    SharedPreferences.getInstance().then((prefs) {
      prefs.setString('project', _project);
    });
  }

  ///
  /// The projects in the database
  Iterable<String> get projects {
    final folder = Directory(databaseFolder);
    if (!folder.existsSync()) {
      return ['Project1', 'Project2', 'Project3'];
    }
    return folder.listSync().whereType<Directory>().map((e) => e.path);
  }

  ///
  /// The current subject
  String _subject = '';
  String get subject => _subject;
  set subject(String value) {
    _subject = value;
    SharedPreferences.getInstance().then((prefs) {
      prefs.setString('subject', _subject);
    });
  }

  ///
  /// The subjects in the current project
  Iterable<String> get subjects {
    final folder = Directory('$databaseFolder/$project');
    if (!folder.existsSync()) {
      return ['Subject1', 'Subject2', 'Subject3'];
    }
    return folder.listSync().whereType<Directory>().map((e) => e.path);
  }

  ///
  /// The last trial name
  String _lastTrial = '';
  String get lastTrial => _lastTrial;
  set lastTrial(String value) {
    _lastTrial = value;
    SharedPreferences.getInstance().then((prefs) {
      prefs.setString('lastTrial', _lastTrial);
    });
  }

  ///
  /// The trials in the current subject
  Iterable<String> get trials {
    final folder = Directory('$databaseFolder/$project/$subject');
    if (!folder.existsSync()) {
      return [];
    }
    return folder.listSync().whereType<File>().map((e) => e.path);
  }

  ///
  /// Check if a trial exists
  bool trialExists(String trial) {
    return Directory(savePath).existsSync();
  }

  ///
  /// Save path to the last trial
  String get savePath => '$databaseFolder/$project/$subject/$lastTrial.walk';

  ///
  /// Generate the structure for the database up to the subject folder
  void createDatabaseStructure() {
    final folder = Directory(savePath);
    if (!folder.existsSync()) {
      folder.createSync(recursive: true);
    }
  }
}

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:frontend/managers/database_manager.dart';

class SaveTrialDialog extends StatefulWidget {
  const SaveTrialDialog({super.key});

  @override
  State<SaveTrialDialog> createState() => _SaveTrialDialogState();
}

enum SaveLevel {
  notPossible,
  possible,
  overwrite;

  @override
  String toString() {
    switch (this) {
      case SaveLevel.notPossible:
        return 'Save';
      case SaveLevel.possible:
        return 'Save';
      case SaveLevel.overwrite:
        return 'Overwrite';
    }
  }
}

class _SaveTrialDialogState extends State<SaveTrialDialog> {
  late SaveLevel _saveLevel = SaveLevel.notPossible;

  final _databaseFocusNode = FocusNode();
  final _projectNameFocusNode = FocusNode();
  final _subjectNameFocusNode = FocusNode();
  final _trialNameFocusNode = FocusNode();

  final _databaseFolderController =
      TextEditingController(text: DatabaseManager.instance.databaseFolder);
  final _projectNameController =
      TextEditingController(text: DatabaseManager.instance.project);
  final _subjectNameController =
      TextEditingController(text: DatabaseManager.instance.subject);
  late final _trialNameController =
      TextEditingController(text: DatabaseManager.instance.lastTrial);

  @override
  void initState() {
    _projectNameController.addListener(() => _updateSaveLevel());
    _subjectNameController.addListener(() => _updateSaveLevel());
    _trialNameController.addListener(() => _updateSaveLevel());

    WidgetsBinding.instance.addPostFrameCallback((_) => _updateSaveLevel());
    super.initState();
  }

  void _updateSaveLevel() async {
    // It cannot be ready to save if all the fields are not filled
    if (_databaseFolderController.text.isEmpty ||
        _projectNameController.text.isEmpty ||
        _subjectNameController.text.isEmpty ||
        _trialNameController.text.isEmpty) {
      if (mounted) setState(() => _saveLevel = SaveLevel.notPossible);
      return;
    }
    final trialName = '${_databaseFolderController.text}/'
        '${_projectNameController.text}/'
        '${_subjectNameController.text}/'
        '${_trialNameController.text}';

    // Mark the trial as must be overwritten if it already exists
    _saveLevel = DatabaseManager.instance.trialExists(trialName)
        ? SaveLevel.overwrite
        : SaveLevel.possible;

    if (mounted) setState(() {});
  }

  void _onCancel() {
    Navigator.of(context).pop(false);
  }

  void _onSave() {
    final db = DatabaseManager.instance;
    db.project = _projectNameController.text;
    db.subject = _subjectNameController.text;
    db.lastTrial = _trialNameController.text;

    db.createDatabaseStructure();
    Navigator.of(context).pop(true);
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('Save Trial'),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Row(
            children: [
              Expanded(
                child: TextFormField(
                  controller: _databaseFolderController,
                  focusNode: _databaseFocusNode,
                  decoration:
                      const InputDecoration(labelText: 'Database folder'),
                  onChanged: (value) => _databaseFolderController.text =
                      DatabaseManager.instance.databaseFolder,
                ),
              ),
              IconButton(
                onPressed: () async {
                  final result = await FilePicker.platform.getDirectoryPath(
                      dialogTitle: 'Select the database folder',
                      initialDirectory: DatabaseManager.instance.databaseFolder,
                      lockParentWindow: true);

                  if (result == null) return;
                  DatabaseManager.instance.databaseFolder = result;
                },
                icon: const Icon(Icons.folder),
              ),
            ],
          ),
          const SizedBox(height: 10),
          _AutoFolderCompleter(
            title: 'Project folder',
            elements: DatabaseManager.instance.projects,
            controller: _projectNameController,
            focusNode: _projectNameFocusNode,
          ),
          const SizedBox(height: 10),
          _AutoFolderCompleter(
            title: 'Subject folder',
            elements: DatabaseManager.instance.subjects,
            controller: _subjectNameController,
            focusNode: _subjectNameFocusNode,
          ),
          const SizedBox(height: 10),
          TextFormField(
            controller: _trialNameController,
            focusNode: _trialNameFocusNode,
            decoration: const InputDecoration(labelText: 'Trial'),
          ),
        ],
      ),
      actions: [
        TextButton(onPressed: _onCancel, child: const Text('Cancel')),
        TextButton(
            onPressed: _saveLevel == SaveLevel.notPossible ? null : _onSave,
            child: Text(_saveLevel.toString())),
      ],
    );
  }
}

class _AutoFolderCompleter extends StatelessWidget {
  const _AutoFolderCompleter(
      {required this.title,
      required this.elements,
      required this.controller,
      required this.focusNode});

  final String title;
  final Iterable<String> elements;
  final TextEditingController controller;
  final FocusNode focusNode;

  @override
  Widget build(BuildContext context) {
    return RawAutocomplete<String>(
      textEditingController: controller,
      focusNode: focusNode,
      optionsBuilder: (controller) => elements
          .where((name) =>
              name.toLowerCase().contains(controller.text.toLowerCase()))
          .toList()
        ..sort((a, b) => a.compareTo(b)),
      fieldViewBuilder: (context, controller, node, onSubmitted) {
        return TextFormField(
          controller: controller,
          focusNode: node,
          onFieldSubmitted: (value) => onSubmitted(),
          decoration: InputDecoration(labelText: title),
        );
      },
      optionsViewBuilder: (context, onSelected, options) {
        return Align(
          alignment: Alignment.topCenter,
          child: Material(
            elevation: 4.0,
            child: SizedBox(
              height: 200,
              child: ListView.builder(
                  padding: const EdgeInsets.all(8),
                  itemCount: options.length,
                  itemBuilder: (context, index) {
                    final name = options.elementAt(index);
                    return ListTile(
                      title: Text(name),
                      onTap: () {
                        onSelected(name);
                      },
                    );
                  }),
            ),
          ),
        );
      },
    );
  }
}

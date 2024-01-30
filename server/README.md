# Lokomat FES - Stimulating the rehabilitation

`lokomat_fes` is a piece of software designed to help clinicians during physical rehab on the Lokomat using functional electrical stimulation.

## Status

| Type | Status |
|---|---|
| License | <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/license-MIT-success" alt="License"/></a> |
| Continuous integration | [![Build status](https://github.com/cr-crme/lokomat_fes/actions/workflows/run_tests.yml/badge.svg)](https://github.com/cr-crme/lokomat_fes/actions) |
| Code coverage | [![codecov](https://codecov.io/gh/cr-crme/lokomat_fes/graph/badge.svg?token=D4HAID52MH)](https://codecov.io/gh/cr-crme/lokomat_fes) |

# Table of Contents 

- [Lokomat FES - Stimulating the rehabilitation](#lokomat-fes---stimulating-the-rehabilitation)
  - [Status](#status)
- [Table of Contents](#table-of-contents)
- [How to install](#how-to-install)
  - [Installing from the sources (For Linux, Mac, and Windows)](#installing-from-the-sources-for-linux-mac-and-windows)
  - [Conda](#conda)
    - [Windows](#windows)
  - [Dependencies](#dependencies)
  - [Using vscode](#using-vscode)
    - [Choosing the right interpreter](#choosing-the-right-interpreter)
    - [Extensions](#extensions)
    - [Configuration files](#configuration-files)
- [Troubleshooting](#troubleshooting)
- [Citing](#citing)


# How to install 
## Installing from the sources (For Linux, Mac, and Windows)
From the root directory simply run the command `pip install .`.

## Conda
The current project uses conda to manage its dependencies.
If you do not have conda installed, please follow the instructions on the [conda website](https://docs.conda.io/projects/conda/en/latest/user-guide/install/).

### Windows
For Windows users, you will need to initialized conda. 
When using `vscode` the chances are that the terminal you are using is `powershell`.
In the terminal tab of vscode, run the following command: `conda init powershell`.

You may face the error `conda is not recognized as an internal or external command, operable program or batch file.`.
If it is the case, you will need to add conda to your path.
To do so, find the path to your conda installation (usually `C:\Users\{USERNAME}\anaconda3\Scripts\conda.exe`) and add it to your path environment (without the `conda.exe` part).
If you do not know how to do so, please follow the instructions on the [Microsoft website](https://docs.microsoft.com/en-us/previous-versions/office/developer/sharepoint-2010/ee537574(v=office.14)?redirectedfrom=MSDN).


## Dependencies
`lokomat_fes` relies on several libraries. 

Here is a list of all direct dependencies (meaning that some dependencies may require other libraries themselves):  
[Python](https://www.python.org/)

To install them, you simply have to create the conda environment from the provided file. 
```bash
conda env create -f environment.yml
```

Alternatively, you can install the dependencies manually by running the following commands:
```bash
conda install numpy nidaqmx-python pip pytest -cconda-forge
pip install crccheck colorama pyserial
```

`lokomat_fes` also need `pyScienceMode`. 
This dependency should be downloaded when git-cloning the current repository, assuming you initialized the submodule.
Once it is done, please navigate to `{ROOT}/external/pyScienceMode` and follow the install instruction.

## Using vscode
If you are using vscode, there is default configuration files that you can use, but you will need to install some extensions first.

### Choosing the right interpreter
First, you need to select the right interpreter for the project.
To do so, open the command palette (`Ctrl+Shift+P`) and search for `Python: Select Interpreter`.
Then, select the interpreter that is in the conda environment you created earlier (`lokomat_fes` if you kept the default name).

### Extensions
Now, you will need to install the vscode extensions `ms-python.python` and `ms-python.black-formatter`.
To do so, open the command palette (`Ctrl+Shift+P`) and search for `Extensions: Install Extensions`.
Then, search for the extensions and install them.

Once it is done, you will need to configure the `black-formatter` extension.
To do so, open the preferences (`Ctrl+,`) and search for `Black Formatter` (which should be in the `Extensions` tab).
Then, in the `Black-formatters: Args` option, add the following: `-l120`.

### Configuration files
Finally, you will need to copy the default configuration files.
First, make a copy of all the `*.default` files from the `{ROOT}/.vscode` folder in the same folder (`{ROOT}/.vscode`).
Rename the copies so they have the exact same name as the original files, but without the `.default` extension.
For example, `settings.json.default` should be renamed to `settings.json`.
Unless you want to change the default settings, you should be good to go.

Assuming all the dependencies are installed (see [Dependencies](#dependencies)), you should now be able to run the main script and the tests properly from now on (by pressing `F5` to run the `main.py` or by running the tests from the test tab on the left-hand side of the screen).
Please note that you may need to restart vscode or even the computer for the changes to take effect.

# Troubleshooting
Despite our best efforts to assist you with this long Readme and several examples, you may experience some problems with `lokomat_fes`.
Fortunately, this troubleshooting section will guide you through solving some known issues.

There is no troubleshooting so far.

# Citing
If you use `lokomat_fes`, we would be grateful if you could cite it as follows:
```bibtex
@misc{cherni2024lokoatfes,
  title={Lokomat FES - Stimulating the rehabilitation},
  author={Michaud, Benjamin and Cherni, Yosra},
  year={2024},
  howpublished ={\url{http://github/cr-crme/lokomat_fes}}
}
```

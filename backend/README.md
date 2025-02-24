# Neurobiomech software - Stimulating the rehabilitation

`neurobiomech software` is a piece of software designed to help clinicians during physical rehab using functional electrical stimulation.

## Status
| Type | Status |
|---|---|
| License | <a href="https://opensource.org/licenses/MIT"><img src="https://img.shields.io/badge/license-MIT-success" alt="License"/></a> |
| Continuous integration | [![Build status](https://github.com/LabNNL/neurobiomech_software/actions/workflows/run_tests.yml/badge.svg)](https://github.com/LabNNL/neurobiomech_software/actions) |
| Code coverage | [![codecov](https://codecov.io/gh/LabNNL/neurobiomech_software/graph/badge.svg?token=D4HAID52MH)](https://codecov.io/gh/LabNNL/neurobiomech_software) |

# Table of Contents 
- [Neurobiomech software - Stimulating the rehabilitation](#neurobiomech-software---stimulating-the-rehabilitation)
  - [Status](#status)
- [Table of Contents](#table-of-contents)
- [How to install](#how-to-install)
  - [Install dependencies](#install-dependencies)
  - [Compile the project](#compile-the-project)
    - [From the console](#from-the-console)
    - [Using vscode](#using-vscode)
    - [Dependencies](#dependencies)
    - [CMake](#cmake)
- [How to use](#how-to-use)
  - [Server side](#server-side)
  - [Client side](#client-side)
    - [Connexion](#connexion)
    - [Commands](#commands)
    - [Responses](#responses)
    - [Passing extra data](#passing-extra-data)
    - [Deserialize the data](#deserialize-the-data)
- [How to contribute](#how-to-contribute)
- [Graphical User Interface (GUI)](#graphical-user-interface-gui)
- [Documentation](#documentation)
- [Troubleshoots](#troubleshoots)
- [Cite](#cite)

# How to install
To use the `neurobiomech software` backend, you must compile the source code yourself.

## Install dependencies

Please refer to the [dependencies](#dependencies) section to know what you need to install before compiling the project.
This should install all the required dependencies to compile the project, including the tools to compile the project.

## Compile the project

### From the console

From the root of the project, create a build folder and go into it:
```bash
mkdir build
cd build
```

Then, you can compile the project using the following command:
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=path/to/install -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_DOC=ON
make
make install
```

### Using vscode

If you are using vscode, you can use the `CMake Tools` extension to compile the project. You can find the `CMake Tools` extension [here](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools).

Once you have installed the extension (and after having the conda environment prepared), you can copy-paste the `.vscode/settings.json.default` file to `.vscode/settings.json` and then modify the `DCMAKE_INSTALL_PREFIX` by replacing `{PATH_TO_INSTALL_DIR}` by the path to the conda environment, e.g. `"-DCMAKE_INSTALL_PREFIX=C:/Anaconda3/envs/stimalker"`.

Then, you can open the project in vscode and then press `Ctrl+Shift+P` and then type `CMake: Configure` and then `CMake: Build`. This will compile the project.


### Dependencies
`neurobiomech software` relies on some other libraries that one must install prior to compiling. Fortunately, all these dependencies are also hosted on the *conda-forge* channel of Anaconda. Therefore creating (and keeping up-to-date) a conda environment should allow to install everything you need to compile
```bash
conda env create -f environment.yml  # This will create a conda environment called "neurobio"
conda activate neurobio
```
or 
```bash
conda env update -f environment.yml  # This will update the conda environment called "neurobio"
conda activate neurobio
```

### CMake
`neurobiomech software` comes with a CMake (https://cmake.org/) project. If you don't know how to use CMake, you will find many examples on Internet. The main variables to set are:

> `CMAKE_INSTALL_PREFIX` Which is the `path/to/install` `neurobio` in. This `path/to/install` should point the base path of the environment returned by `conda env list`. 
>
> `BUILD_BINARIES` If you want (`ON`) or not (`OFF`) to build the binaries of the project. Default is `ON`. Please note that to get the server, you need to build the binaries.
> 
> `CMAKE_BUILD_TYPE` Which type of build you want. Options are `Debug`, `RelWithDebInfo`, `MinSizeRel` or `Release`. This is relevant only for the build done using the `make` command (i.e. not on Windows). Please note that the software will be slow if you compile it without any optimization (i.e. `Debug`). 
>
> `BUILD_TESTS` If you want (`ON`) or not (`OFF`) to build the tests of the project. Please note that this will automatically download gtest (https://github.com/google/googletest). Default is `OFF`.
>
> `BUILD_DOC` If you want (`ON`) or not (`OFF`) to build the documentation of the project. Default is `OFF`.
>


# How to use

## Server side

Once compiled, you only need to run the `main_server` binary. 

The default ports are :
- command = 5000
- response = 5001
- live data = 5002
- live analyses = 5003

Please note that you may have to open the ports on your firewall to allow the communication.

You can change the ports by recompiling the project and passing the TcpServer in `main_server.cpp`. 

## Client side

The communication protocol is in two times. First, all the connexion to the server must be made, then the client is allowed to send and receive data from the server.

### Connexion

1. First all the sockets (command = [5000], response = [5001], live data = [5002], live analyses = [5003]) must be connected to their respective ports. Please note, each socket has 5 seconds to connect after the previous has connected. If the client fails to connect, the server will close the connexion (closes all the sockets open) and the client will have to reconnect.
2. Once all the sockets are connected, the client must send a HANDSHAKE command (see below) to the server. The server will respond with a OK or a not on the command socket. If the server responds with a not OK, the client must close all the sockets and reconnect.
3. Once the connexion is established, the client can send commands to the server.

### Commands

All the commands are sent over the command socket. All the commands must be formatted the same way.

The command is made of exactly two 4 bytes, each litten-endian. 
  - The first 4 bytes is the version of the protocol. The current version is 1. 
  - The second 4 bytes is the command. The list of commands is below.

      HANDSHAKE =                   0
      CONNECT_DELSYS_ANALOG =      10
      CONNECT_DELSYS_EMG =         11
      CONNECT_MAGSTIM =            12
      ZERO_DELSYS_ANALOG =         40
      ZERO_DELSYS_EMG =            41
      DISCONNECT_DELSYS_ANALOG =   20
      DISCONNECT_DELSYS_EMG =      21
      DISCONNECT_MAGSTIM =         22
      START_RECORDING =            30
      STOP_RECORDING =             31
      GET_LAST_TRIAL_DATA =        32
      ADD_ANALYZER =               50
      REMOVE_ANALYZER =            51
      FAILED =                    100

    For example, if you want to send a DISCONNECT_DELAYS_EMG command, you must send the following 8 bytes:
    ```
    01 00 00 00 15 00 00 00
    ```

Some of the commands are expected to pass extra data. The extra data is always sent over the live data socket. The extra data is always passed over the response socket after the command has received an OK response. To see how to format the extra data, please refer to the documentation of the command below.

Some of the commands expect a response from the server. The response is always sent over the response socket BEFORE sending the acknowledgement on the command port. The response is a header with a serialized version of the data. The header is always the exact same as the Response (see Responses section below) with the last 4 bytes being the size of the data (instead of OK or NOK). Internally, the server will send the data over two calls (the header, then the data). The client must be able to handle this. To see how to deserialize the response, please refer to the documentation of the command below.

### Responses

The server will always respond to a command on the command socket OK or a NOK before anything else happens. If the server responds with a NOK, the client must assume the request has failed and must redo it. 

The response is made of exactly three parts, one 4 bytes, one 8 bytes and one 4 bytes. The endian depends on the server architechture (as memcpy is used). Most
of the time, the server will be little-endian.
  - The first part of 4 bytes is the version of the protocol. The current version is 1. 
  - The second part of 8 bytes is the timestamp in millisecond since UNIX epoch (i.e. 1st of January 1970).
  - The third part of 4 bytes is the response. The list of responses is below.

      NOK = 0
      OK = 1

    For example, if you sent a command that failed (NOK = 1) at 3PM on the 24th of June 1995 (i.e. time since epoch = 0xBB33909E00), the server will respond with the following 16 bytes: (I added / to make it easier to read, but they would obviously not be in the response)
    ```
    01 00 00 00 / 00 9E 90 33 BB 00 00 00 / 01 00 00 00
    ```

### Passing extra data

TODO

### Deserialize the data

# How to contribute
You are very welcome to contribute to the project! There are to main ways to contribute. 

The first way is to actually code new features for `neurobiomech software`. The easiest way to do so is to fork the project, make the modifications and then open a pull request to the main project. Don't forget to add your name to the contributor in the documentation of the page if you do so!

The second way is to open issues to report bugs or to ask for new features. I am trying to be as reactive as possible, so don't hesitate to do so!

# Graphical User Interface (GUI)
See the `frontend` folder

# Documentation
The documentation is automatically generated using Doxygen (http://www.doxygen.org/). You can compile it yourself if you want (by setting `BUILD_DOC` to `ON`). 

# Troubleshoots
Despite our best effort to make a bug-free software, `neurobiomech software` may fails sometimes. If it does, please refer to the section below to know what to do. We will fill this section with the issue over time.


# Cite
If you use `neurobiomech software`, we would be grateful if you could cite it as follows:

```

@misc{cherniNeurobio2021,
  title = {The Neurobiomech Software: Stimulating the rehabilitation},
  shorttitle = {neurobio},
  author = {Michaud, Benjamin and Cherni, Yosra},
  date = {2024-01-01},
  url = {https://github.com/LabNNL/neurobiomech_software},
  urldate = {2024-01-01},
  langid = {english}
}
```

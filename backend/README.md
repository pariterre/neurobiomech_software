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
    - [Client command packets](#client-command-packets)
    - [Passing extra data to the server](#passing-extra-data-to-the-server)
    - [Server OK/NOK response packets](#server-oknok-response-packets)
    - [Server data response packets](#server-data-response-packets)
    - [Serialize the extra data](#serialize-the-extra-data)
      - [ADD\_ANALYZER](#add_analyzer)
      - [REMOVE\_ANALYZER](#remove_analyzer)
    - [Deserialize the data response](#deserialize-the-data-response)
      - [GET\_LAST\_TRIAL\_DATA](#get_last_trial_data)
      - [Live data](#live-data)
      - [Live analyses](#live-analyses)
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

The communication protocol is in two steps. First, all the connexion to the server must be made, then the client is allowed to send and receive data from the server.

### Connexion

1. First all the sockets (command = [5000], response = [5001], live data = [5002], live analyses = [5003]) must be connected to their respective ports. Please note, each socket has 5 seconds to connect after the previous has connected. If the client fails to connect, the server will close the connexion (closes all the sockets) and the client will have to reconnect.
2. Once all the sockets are connected, the client must send a HANDSHAKE command (see `Client command packets` below) to the server. The server will respond with an OK or a NOK on the command socket (see `Server OK/NOT response packets` section below). If the server responds with a NOK, the client must close all the sockets and reconnect.
3. Once the connexion is established, the client can send commands to the server.

### Client command packets

All the commands are sent over the command socket. All the commands must be formatted the same way.

The command is made of exactly two 4 bytes, each little-endian. 
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

    For example, if you want to send a DISCONNECT_DELAYS_EMG command, you must send the following 8 bytes (I added / to make it easier to separate the two sections of the command, but they would obviously not be in the command):
    ```
    01 00 00 00   /   15 00 00 00
    ```

Some of the commands are expected to pass extra data. To see how to format the extra data, please refer to the `Passing extra data to the server` section below.

Some of the commands expect a data response from the server. To see how the data is formatted, please refer to the `Server data response packets` section below.

### Passing extra data to the server

If some extra data must be sent to the server in conjonction with a command, the client must first send the command, then they must wait for the OK response on the command socket. Only then can they send the extra data on the response socket. 

The format of the extra data is made of three part consisting of an 8 bytes header followed by the serialized data:
  - The first 4 bytes is the version of the protocol, in little-endian. The current version is 1. 
  - The second 4 bytes is the size of the data in bytes, in little-endian.
  - The third part is the data itself, as a json string.

Finally a second OK/NOK response will be sent on the command socket to indicate if the extra data was correctly received by the server.

The commands that expect to send extra data are:
      ADD_ANALYZER =               50
      REMOVE_ANALYZER =            51

To see how to serialize the data, please refer to the `Serialize the extra data` section below.

### Server OK/NOK response packets

The server will always respond to a command on the command socket an OK or a NOK on that same port. If the server responds with a NOK, the client must assume the request has failed and must redo it if needed.

The response packets is made of exactly three parts, one of 4 bytes, one of 8 bytes and one of 4 bytes. all in little-endian.
  - The first part of 4 bytes is the version of the protocol. The current version is 1. 
  - The second part of 8 bytes is the timestamp in millisecond since UNIX epoch (i.e. 1st of January 1970).
  - The third part of 4 bytes is the response. The possible responses are:

      NOK = 0
      OK = 1

    For example, if you requested a command that failed (i.e. 3rd part being NOK = 1) at 3PM on the 24th of June 1995 (i.e. 2nd part being the time since epoch of that date time = 0xBB33909E00), the server will respond with the following 16 bytes: (I added / to make it easier to read, but they would obviously not be in the response)
    ```
    01 00 00 00   /   00 9E 90 33 BB 00 00 00   /   01 00 00 00
    ```

### Server data response packets

If a command is expected to respond with data, the server will send the data over the response socket. The data is always sent over the response socket BEFORE the server sends its OK/NOK response on the command port. 

The format of the data response is made of three part consisting of an 8 bytes header followed by the serialized data:
  - The first 4 bytes is the version of the protocol, in little-endian. The current version is 1. 
  - The second 4 bytes is the size of the data in bytes, in little-endian.
  - The third part is the data itself, as a json string.

Internally, the server will send the data over two calls (the header, then the data). The client must be able to handle this. 

To see how to deserialize the response, please refer to the `Deserialize the data response` section below.

### Serialize the extra data

Each of the command that expects extra data will have a different format. That said, the format of the data is always a json string.

In this section, if something is written in capital letters, it means this thing must be replaced by the actual value. Important, if there are "", they should be kept in the json string.
  
  - If it starts by "PROVIDE_" it means that the user must provide an arbitrary value of their choice
    - If it is followed by "PROVIDE_STR_", it means that the user must provide a string
    - If it is followed by "PROVIDE_INT_", it means that the user must provide an integer
    - If it is followed by "PROVIDE_FLOAT_", it means that the user must provide a float
    - If a list is expected, it is written as [VALUE1, VALUE2, ...], with the ... meaning that the user can add as many values as they want
  - If it starts by "SELECT_" it means that the user must select the value from a list of possible values written right later in the section


The commands that expect to serialize data are:
      ADD_ANALYZER =               50
      REMOVE_ANALYZER =            51

#### ADD_ANALYZER

The ADD_ANALYZER command expects all the parameters of the analyzer to be passed. The format of the json string is as follows:

```json
{
  "name" : "PROVIDE_STR_NAME_OF_THE_ANALYZER",
  "analyzer_type" : "SELECT_ANALYZER_TYPE",
  "time_reference_device" : "SELECT_DEVICE",
  "learning_rate" : PROVIDE_FLOAT_LEARNING_RATE,
  "initial_phase_durations" : [PROVIDE_INT_DURATION_FIRST_EVENT, PROVIDE_INT_DURATION_SECOND_EVENT, ...],
  "events" : [
    {
      "name" : "PROVIDE_STR_NAME_OF_THE_FIRST_EVENT",
      "previous" : "PROVIDE_STR_NAME_OF_THE_PREVIOUS_EVENT",
      "start_when" : [
        {
          "type": "SELECT_START_WHEN_TYPE",
          "device" : "SELECT_DEVICE",
          "channel" : PROVIDE_INT_CHANNEL,
          "comparator" : "SELECT_COMPARATOR",
          "value" : PROVIDE_FLOAT_VALUE
        },
        {
          "type": "SELECT_START_WHEN_TYPE",
          "device" : "SELECT_DEVICE",
          "channel" : PROVIDE_INT_CHANNEL,
          "direction" : "SELECT_DIRECTION"
        }, 
        ...
      ]
    }, 
    ...
  ]
}
```
Notes:
  - The `time_reference_device` is the device that will be used as the reference for the time of the prediction relative to the data.
  - The `learning_rate` is a float between 0 and 1 (that is not actually enforced, but providing values outside this range does not make much sense)that is used to update the weights of the model.
  - The `initial_phase_durations` is a list of positive integers of time in milliseconds. The list must have the same length as the number of events.
  - There can be as many `events` as needed. But the PROVIDE_STR_NAME_OF_THE_PREVIOUS_EVENT must be the name of an event that exist in the list of events (it can be declared after), including itself. 
  - The `start_when` section can have as many elements as needed, and not all the SELECT_START_WHEN_TYPE are needed. For example, if the SELECT_START_WHEN_TYPE is only one "threshold", the "direction" one is not needed. However, the elements composing each "start_when" depends on the SELECT_START_WHEN_TYPE (e.g. "direction" does not have a "comparator" and "value" element).

The SELECT_ANALYZER_TYPE can be one of the following:
  - "cyclic_timed_events"

The SELECT_DEVICE can be one of the following:
  - "DelsysAnalogDataCollector"
  - "DelsysEmgDataCollector"

The SELECT_START_WHEN_TYPE can be one of the following:
  - "threshold"
  - "direction"

The SELECT_COMPARATOR can be one of the following:
  - ">"
  - "<"
  - ">="
  - "<="
  - "=="
  - "!="

The SELECT_DIRECTION can be one of the following:
  - "positive"
  - "negative"

#### REMOVE_ANALYZER

The remove analyzer command expects the name of the analyzer to be removed. The format of the json string is as follows:

```json
{
  "analyzer" : "PROVIDE_STR_NAME_OF_THE_ANALYZER"
}
```
Notes:
  The `analyzer` must be the name of an analyzer that exists in the list of analyzers (i.e. it must have been added before using the ADD_ANALYZER command).

### Deserialize the data response

#### GET_LAST_TRIAL_DATA

The GET_LAST_TRIAL_DATA command expects the server to respond with the last trial data. The format of the json string is as follows:

```json
{
  "INT_DEVICE_INDEX" : {
    "data" : {
      "data": [
        [INT_TIMESTAMP, 
        [FLOAT_DATA_CHANNEL1, FLOAT_DATA_CHANNEL2, ...], 
        null],
        ...
      ],
      "start_time" : INT_START_TIME,
    },
    "name": "STR_DEVICE_NAME"
  }, 
  ...
}
```
Notes:
  - The `INT_DEVICE_INDEX` is the index of the device in the list of devices. This is a unique value for each device, but is not important for the user.
  - The `INT_TIMESTAMP` is the timestamp of the data in microseconds since the `start_time`.
  - The `FLOAT_DATA_CHANNEL1, FLOAT_DATA_CHANNEL2, ...` are the data of the channels of the device at that time, the number of which depends on the device.
  - The final `null` is a reserve space for passing extra data, but is not used in the trial data.
  - The `INT_START_TIME` is the time in microseconds since the UNIX epoch (i.e. 1st of January 1970) of the start of the trial.
  - The `STR_DEVICE_NAME` is the name of the device that the data is coming from. Contrary to the `INT_DEVICE_INDEX`, this value is not necessarily unique.
  - If more than one device is connected, there will be more than one `INT_DEVICE_INDEX` in the response.

#### Live data

The live data socket will start streaming data as soon as the server connects at least one Data collector. The data will be sent as soon as it is available. The format of the data is the same as the `GET_LAST_TRIAL_DATA` command.

Please note, internally the live data are not stored. Therefore, when sent, the data consist only of the last predicatable frame. This means thata lot of data that could be predicted are actually skipped. It is up to the client to merge the data with the previous ones.

#### Live analyses

The live analyses socket will start streaming data as soon as the server connects at least one Analyzer and has the required Data collector to actually predicts something. The format of the analyses are as follow:

```json
{
  "data" : {
    "STR_ANALYZER_NAME" : [
      INT_TIMESTAMP,
      [FLOAT_PREDICTION1, FLOAT_PREDICTION2, ...],
      {
        "current_phase" : INT_CURRENT_PHASE,
        "has_changed_phase" : BOOL_HAS_CHANGED_PHASE
      }
    ],
    ...
  },
  "starting_time" : INT_STARTING_TIME
}
```
Notes:
  - The `STR_ANALYZER_NAME` is the name of the analyzer that made the prediction. It corresponds to the `name` field of the analyzer when invoking the `ADD_ANALYZER` command.
  - The `INT_TIMESTAMP` is the timestamp of the prediction in microseconds since the `starting_time`.
  - The `FLOAT_PREDICTION1, FLOAT_PREDICTION2, ...` are the predictions of the analyzer at that time, the number of which depends on the analyzer, but is usually only composed of one value.
  - The `INT_CURRENT_PHASE` is the current phase of the prediction, corresponding of the `events` in the analyzer. This is an integer that is between 0 and the number of phases (delimitered by the events) of the analyzer minus one.
  - The `BOOL_HAS_CHANGED_PHASE` is a boolean that is true if the phase has changed since the last prediction, false otherwise. This is a redundant information, as one can infer it from the `INT_CURRENT_PHASE` and the previous prediction, but is provided for convenience.
  - The `INT_STARTING_TIME` is the time in microseconds since the UNIX epoch (i.e. 1st of January 1970) of the start of the live analyses.

Please note, internally the live analyses are stored in a rolling vector. When sent, the data are unrolled, meaning that the data are sent in chronological order. However, the data are not filtered. This means that previously sent data may (actually will) be sent again. It is up to the client to keep track of the data they have received and filter out the data they already have.

Please also note, not all the data frame will be evaluated but only once each 50ms. This means that the data will be sent at most every 50ms.

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

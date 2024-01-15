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
  - [Dependencies](#dependencies)
- [Troubleshooting](#troubleshooting)
- [Citing](#citing)


# How to install 
## Installing from the sources (For Linux, Mac, and Windows)
From the root directory simply run the command `pip install .`.

## Dependencies
`lokomat_fes` relies on several libraries. 

Here is a list of all direct dependencies (meaning that some dependencies may require other libraries themselves):  
[Python](https://www.python.org/)

To install them, you simply have to create the conda environment from the provided file. 
```bash
conda env create -f environment.yml
```

Alternatively, you can install the dependencies manually by running the following command:
```bash
conda install -cconda-forge
```

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

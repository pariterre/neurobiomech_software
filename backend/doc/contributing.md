# Contributing to `neurobiomech software`
All contributions, bug reports, bug fixes, documentation improvements, enhancements, and ideas are welcome.
We recommend going through the list of [`issues`](https://github.com/LabNNL/neurobiomech_software/issues) to find issues that interest you, preferable those tagged with `good first issue`.
You can then get your development environment setup with the following instructions.

## Forking `neurobiomech_software`

You will need your own fork to work on the code.
Go to the [neurobiomech software project page](https://github.com/LabNNL/neurobiomech_software) and hit the `Fork` button.
You will want to clone your fork to your machine:

```bash
git clone https://github.com/your-user-name/neurobiomech_software.git
```

## Creating and activating conda environment

Before starting any development, we recommend that you create an isolated development environment. 
The easiest and most efficient way is to use an anaconda virtual environment and to create it based on the `environment.yml` file. 

- Install [miniconda](https://conda.io/miniconda.html)
- `cd` to the neurobiomech software source directory
- Install neurobiomech software dependencies with:

```bash
conda env create -f environment.yml
conda activate neurobio
```

## Implementing new features

Before starting to implement your new awesome feature, please discuss the implementation with the code owner to prevent any clashing with some other current developments. 
It is also a good idea to check the current opened pull-request to not redo something currently being developed. 
If your feature is mentioned in the issue section of GitHub, please assign it to yourself.
Otherwise, please open a new issue explaining what you are currently working on (and assign it to yourself!).

As soon as possible, you are asked to open a pull-request (see below) with a short but descriptive name. 
To tag that a pull-request is still in development, please add `[WIP]` at the beginning of the name.
Send commits that are as small as possible; 1 to 10 lines is probably a good guess, with again short but descriptive names. 
Be aware of the review done by the maintainers, they will contain useful tips and advices that should be integrated ASAP. 
Once you have responded to a specific comment, please respond `Done!` and tag it as resolved.

Make sure you create a unit test with numerical values for comparison.
Please note that if by accident you add a binary file in the history, your pull-request will be rejected and you will have to produce a new pull request free from the binary file. 

When you have completed the implementation of your new feature, navigate to your pull-request in GitHub and select `Pariterre` in the `Reviewers` drop menu. 
At the same time, if you think your review is ready to be merged, remove the `[WIP]` tag in the name (otherwise, your pull-request won't be merged). 
If your pull-request is accepted, there is nothing more to do. 
If changes are required, reply to all the comments and, as stated previously, respond `Done!` and tag it as resolved. 
Be aware that sometimes the maintainer can push modifications directly to your branch, so make sure to pull before continuing your work on the feature.

## Testing your code

Adding tests is required to get your development merged to the master branch. 
Therefore, it is worth getting in the habit of writing tests ahead of time so this is never an issue.
Each time you push to your pull-request, the `neurobiomech software` test suite will run automatically.
However, we strongly encourage running the tests prior to submitting the pull-request.
If you configured your project using CMake and set `BUILD_TEST` to `ON`, the google test toolbox should download and compile itself.
You can thereafter run the test by running the binary `neurobio_tests` (the file will be at different place depending on the OS you are using).

## Convention of coding

`neurobiomech software` tries to follow as much as possible a coherent standard, that is essentially:
  - camelCase;
  - 120 characters per line max;
  - The linting (in vscode) should be set to `ms-vscode.cpptools` (ideally on save)
  - `using namespace` can only be used in the `.cpp` files;
  - `#include` should be sorted alphabetically;
  - attributes shoulde be protected and prefixed by `m_`;

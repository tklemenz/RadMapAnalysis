# Install guide

## Prerequisites

* root
* cmake

## Installation

1. Go to your favorite destination
2. `git clone https://github.com/tklemenz/RadMapAnalysis`
3. source your `thisroot.sh`
4. `source compile.sh`
5. It's that easy

# How to use it

When you want to open a root file containing objects from the framework you need to load the RadMapAnalysis.so shared library.

The easiest way I found for now is to create an alias in your `~/.bashrc` where the library is loaded when you call `root`.
It could look something like this:

`alias radROOT='root -l -e "gSystem->Load(\"/path/to/RadMapAnalysis/build/libRadMapAna.so\");"'`

Then you can simply call radROOT and the objects from the framework are known to root.

# General stuff

* compile.sh should not be changed!
* Executables will be located in the bin folder.
* As the only style guide I would suggest to NOT use tab but only spaces. Otherwise the code looks horrible in different editors...

Please only commit the following files or files in the following folders if changes were made:

	- CMakeLists.txt
	- include
	- src
	- macros
	- README.md

Do NOT commit:

	- bin
	- build


Feel free to not only add macros but also add missing functionality to the classes or create new classes that you think are necessary.

Thanks for contributing!!

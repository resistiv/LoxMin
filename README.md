
[//]: # (Hey!! This document is best viewed on GitHub: https://github.com/resistiv/LoxMin)

# LoxMin
LoxMin is a C implementation of the Lox bytecode virtual machine from the book [*Crafting Interpreters* by Robert Nystrom](https://craftinginterpreters.com/). This project was developed primarily as an assignment for CS 403/503 - Programming Languages (Fall 2023) at the University of Alabama. During development, I mostly followed the direction of the book, changing function and variable names where I saw fit, alongside reorganizing the folder structure of the repository.

LoxMin has been tested on Windows 10 and Ubuntu 22.04.

## Usage
When run without an argument, LM operates as a <abbr title="read-eval-print loop">REPL</abbr> prompt which runs until it encounters an exit code. Otherwise, when given a Lox source file, LM will attempt to execute it and then exit.
```
LoxMin [Lox script]
```

## Building
Building LoxMin requires a C compiler, such as gcc. It can be installed via a package manager, such as ``apt``.
```
sudo apt install build-essential
```
On Windows, it can be installed via Cygwin and mingw/mingw-w64 [here](https://cygwin.com/install.html). Particular instructions for mingw-w64 are available [here](https://www.mingw-w64.org/downloads/#cygwin).

Once installed, clone the repository to a location of your choosing.
```
git clone https://github.com/resistiv/LoxMin.git
```
Navigate to the ``LoxMin`` folder and run the following:
```
make
```
The resulting executable will be built to the ``LoxMin`` folder, from which it can be run.


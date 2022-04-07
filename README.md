# ksh

It's a shell written in C.

## Requirements 

`cmake`, `make` and `gcc` are required to build and run ksh.

## Installation instructions

1. Clone the repo with `git clone git@github.com:akcube/ksh.git`.
2. `cd` into the cloned directory `cd ksh`.
3. Make a build directory and `cd` into it. `mkdir build && cd build`.
4. Run `cmake .. && make -j` to build the binaries.
5. You can now run the shell with `./bin/ksh`.

## Assumptions

1. Commands are only upto 4096 characters long. 
2. History should not be deleted during shell execution. Will still function as normal but history will be lost.
3. All commands **must** end with a `&` or `;` or `\n`
4. I assume proccesses with state R and S only are "Running" processes

### Functionality

- [x] Displays prompt as `<username@system_name:cur_dir>`. `cur_dir` is the directory the shell was launched in.
- [x] Can execute `;` spaced commands and ignores whitespaces and tabs. 
- [x] Builtin commands:
	- [x] `cd`
	- [x] `pwd`
	- [x] `echo`
	- [x] `pinfo`
	- [x] `ls -[al]`
- [x] Can execute system processes in foregroun and background and also keep track of them
- [x] Can repeat commands (even recursively!)
- [x] Implements history
- [x] Implements up arrow and bottom arrow key to access history dynamically
- [x] Input output redirection
- [x] Piping of multiple commands w/ redirection
- [x] `jobs`
- [x] `fg`, `bg` and `sig`
- [x] Signal handlers
- [x] Replay repeats commands in intervals of time t for a period p
- [x] Baywatch command

### File structure
`builtins.c` contains code for the builtin functions, except ls.
`ls.c` contains code for ls.
`error_handlers.c` contains code for the error handlers.
`execute.c` contains code for functions that execute both system and call builtin functions.
`history.c` contains code for pushing logs into history.
`parsing.c` contains code for parsing input lines read from terminal into Command structs and arg string vectors.
`proclist.c` contains code for a doubly linked list that stores the list of active background processes.
`prompt.c` contains code for reading input, up/bottom arrow keys and displaying prompt.
`shell.c` contains the REPL loop.
`signal_handlers.c` contains code for both installing the handlers and the handlers themselves.
`utils.c` contains code for util functions used throughout the code. Noteworthy functions are init which sets up all the basic shell state resources and cleanup which frees resources and saves history to file.
`vector.c` contains code for a string vector object that supports pushback, top, dynamic reallocation for O(1) amortized insertion, and sorting. 

They've been heavily commented and the functions should be mostly self explanatory. 


# ksh

It's a shell

## Usage

1. `Make` & `gcc` must be installed on the machine
2. Run `make` to compile the program. 
3. Run with `./shell`

## Assumptions

1. Commands are only upto 4096 characters long. 
2. History should not be deleted during shell execution. Will still function as normal but history will be lost.
3. All commands **must** end with a `&` or `;` or `\n`

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
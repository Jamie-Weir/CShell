# CShell
Custom Unix Shell

A lightweight Unix-style command-line shell written in C. This project implements core shell functionality including command execution, directory navigation, 
command history, aliases, and environment variable management.

The project was developed to gain practical experience with processes, system calls, 
environment variables, dynamic memory, file handling, and command parsing in C.

Features
Execute external Unix commands using fork() and execvp()
Navigate directories using cd
View and modify the system PATH
Persistent command history
History command execution using:
!! — execute the most recent command
!<number> — execute a specific history entry
!-<number> — execute a command relative to the current history
Create and manage command aliases
Persistent aliases between shell sessions
Alias chaining with protection against infinite alias loops
Built-in help command
Input validation and error handling
Restores the original PATH when the shell exits
Built-in Commands

Command	Description

cd <directory>	Change the current working directory

help	Display available shell commands

getpath	Display the current PATH

setpath <path>	Change the current PATH

history	Display stored command history

!!	Execute the most recent command

!<number>	Execute a numbered history command

!-<number>	Execute a command relative to the current history

alias	Display all configured aliases

alias <name> <command>	Create or replace an alias

unalias <name>	Remove an alias

exit	Exit the shell

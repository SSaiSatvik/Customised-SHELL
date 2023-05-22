# ASSIGNMENT 2 & 3

##  cd
* If incorrect directory is given,then is remain in the same previous directory.
* Works only if one relative/absolute path is given,remain in the same previous directory.
* Will not work if the path is like ~/Desktop,....
* No folder should have spaces ot tabs.

## echo
* Will not remove "" if exsist.

## pwd
* No extra parameters should be passed.

## ls
* If -l flag is there time format is like tue sep 6 time year
* Will work if the path is like ~/Desktop,....
* No folder should have spaces ot tabs.
* No alphabet should be present beside - for flags except a,l.
* If multiple output are there then they are sorted in lexographical order not alphabetical order.
* If multiple parameters are passed,the output is starting with parametername: ....,even if it is a file or folder.
* No "" should be present.
* Hidden folders should not be passed as parameter.

## pinfo
* "+" in process state is not considered for the original process(shell).
* If we try to do pinfo of any completed process,then it gives segfault.
* Only one integer valid argument should be passed.

## discover
* The printed output will always be absolute path in order to reduce confusion when  command is given like discover ./..
* The first line is like root directory which has to be traversed.It will be printed if directories are allowed to print and there is no find.
* Valid directory name should be given in 3rd parameter. 
* Valid parameters should be passed.
* My code will work even if we give absolute or relative path in path.
* Then given path should be a folder only.

## history
* If extra spaces of tabs are there in command,then they are removed in history.

## jobs
* Implemented on with flags either -r or -s.(Both both -r and -s are present it wont work.)
* No extra parameter to be passed.

## sig
* Only 2 parameter to be passed.

## fg
* Only 1 extra parameter to be passed.

## bg
* Only 1 extra parameter to be passed.

## General
* Code will work even if there are no spaces in between alphabet and ; or & on either side.
* Colours are used only ls command.
* The input is typed on the next line of the prompt. 
* If there are multiple parts in the given command seperated by ; or & ,my code considers it as 2 different commands and enter them both.

* If there is any input or outfile redirection then they should present at the last of the command.
* If both output and input redirection is done then space seperation in compulsory.(Ex:-sort "<"filea ">"fileb)
* Should have a new line character at last of input file.

* When control+D is pressed,it will work only if there no running on foreground.
* Control + Z works only when some foreground process other than our terminal is working.

* When running on wsl ,it may create some issues sometimes.
* Ignore warning in terminal.

### COMMANDS
* make
* ./hellomake

* gcc -g newmain.c new.c
* gcc -g func.c main.c
 
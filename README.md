# mini-shell
• I have printed the prompt showing current directory path in ANSI_COLOR_BLUE (Bold).

• The temporary history file is created as “/tmp/history.txt”.

• Supported internal commands: cd, echo, pwd, history, exit
  For echo command: use 'echo abc'
  For exit command: use 'exit' or 'quit' or 'x' (Any one will work)
  
• For environment variables: use 'setenv TERM = vt100'
  for printing, use 'printenv SHELL' or 'echo $SHELL'
  
• Supported external commands: ls, cat, man, which, whereis, date, cal, clear,
                               apropos, more, less, touch, find, cp, mv, rm, 
                               mkdir, rmdir, sort, wc
(All commands supported by execv)

• Single level piping is supported for both internal and external commands.
  Ex: ls -a | wc -c
  
• For redirection, the following cases are supported for both internal and external 
  commands:-
   1. wc <input.txt or wc < input.txt
   2. ls >output.txt or ls > output.txt
   3. ls >>output.txt or ls >> output.txt
   4. wc <input.txt >output.txt or wc < input.txt > output.txt
   5. wc <input.txt >>output.txt or wc < input.txt >> output.txt

• I have also included help command which will print the supported internal commands, 
  allowed piping and redirection.
  For help command: use 'help'

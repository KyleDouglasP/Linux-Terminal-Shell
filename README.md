My Shell
=============================

Author: Kyle Perry

Build Instructions
==================

1. Make sure you are connected to an SSH session on a Rutgers iLabs machine
2. Run `make`; this should create one file
3. Run `./mysh` for interactive mode
4. Run `./mysh <file>` for batch mode

Testing Scenarios
=================

If you would like to run the test scenarios, do the following:

1. Run `make tests`; this will create five files
2. Run the following tests: (Also included in tests.sh)

    Testing pwd & cd:
        1. Make sure pwd returns the correct working directory
        2. cd into the test folder and run pwd again to confirm
        3. Try and run `hello` in the test directory, it should not work
        4. cd .. to go back to the previous directory and then run `hello`, it should work

    Testing Redirection:
        1. Run `hello > output.txt`, this will create a new output.txt file with the correct output
        2. Run `hello2 kyle > output.txt`, this will overrite output.txt with the new output
        3. Run `hello2 > output.txt kyle`, this should perform the same thing as 2
        4. Run `redirecttest < numbers.txt > output.txt` this should cause ouput.txt to be overwritten with the numbers in numbers.txt
        5. Run `redirecttest > output.txt < numbers.txt` this should do the same as 4
        6. Run `hello > output.txt > output2.txt`, this should cause an error as we can't have two outputs
        7. Run `hello < output.txt < input.txt`, this should cause an error as we can't have two inputs

    Testing Piping:
        1. Run `kyle | pipetest`, this should cause the name printed in kyle to not be printed to the terminal, but instead be used in pipetest then pipetest prints to terminal
        2. Run `kyle | pipetest > output.txt`, same as 1 but ouput goes to output.txt
        3. Run `exit David Menendez | pipetest`, this will use David Menendez as the name for pipetest, and then exit terminal

3. Test with any other .c files



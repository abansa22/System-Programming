git, personal and public repositories
The primary objective of this homework is to get you familiarized with the ELF file format and the process of linking executables. The secondary goal of this assignment is to give you some experience using the Gradescope autograder and turnin system.
We covered using git in the first lab, but if you’re still not that familiar with it, this assignment is a second chance to get up to speed. Remember: git gives you warnings and errors for good reason, if it complains at the command line when you run a command, don’t just assume it’s completed correctly!
Setting git to the simple push is a helpful quality of life improvement, this is recommended but not required:
git config --global push.default simple
You should also tell git who you are, otherwise it will complain that you are some anonymous coder. Do this, but use your own name and email. Note that if you do not change your username and email properly, you won’t get credit.
$ git config --global user.name "Copypasting Carol"
$ git config --global user.email copypasta@domain.invalid
The skeleton code for this assignment is available at this link. You must use GitHub classroom to manage your code (the repository created through that link). You will need to submit the code via Gradescope.
Now that you have the skeleton code, you can start coding. You should commit early and often, and push to your remote repository whenever is convenient to back up your work!


Programming environment
This assignment was created in a Debian-derived Linux environment. This assignment is simple enough that any Linux environment with an up to date gcc will be sufficient, including systems{1,2,3,4}.cs.uic.edu. The autograder is written to the correct specification for the autograding environment. Feel free to complete this on a lab machine, a local Linux VM, or elsewhere.


The Programming Part!
This part will give you a quick introduction to using readelf to better understand the linking process.
In this assignment, you must fill hw1.c and hw1.h with code which will:
1.	cause your UIC netID (and nothing else) to be printed on the first line of output when the program is run.
2.	cause gcc -Wall hw1.c to issue zero warnings and zero errors.
3.	cause the output of readelf -s hw1.o to have an identical number of symbol table entries as shown below (25 entries).
4.	cause the output of readelf -s hw1.o to have identical values in the bold sections of the output below:
You can perform the symbol table check by running make test within the hw1 directory. A full credit solution will have output.

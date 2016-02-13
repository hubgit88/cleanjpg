A small simple command line program ( written in c++ ) that will remove unneeded data from inside of jpg files.

The program source code along with a windows exe is the svn repository. It should also compile on other systems like linux, unix and macintosh without any changes. Just use the attached makefile.

I wrote it so that images uploaded to my website are smaller and have all unused data removed from inside them. I have an automated upload system for sending new pictures to my website and before they are uploaded it runs cleanjpg on them first.

The program just takes one operand and that is the path ( or folder ) where the jpg images are located. It scans the directory and looks for filenames ending in ".jpg" (or ".JPG).

Enjoy,
Kevin
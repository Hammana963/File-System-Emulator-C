This code is written in C and emulates a File System. The program supports directories and "regular" files, 
allows a user to navigate the file system (change between directories), and allows the user to modify the file system (e.g., add/move directories and files).

The code was entirely authored by myself, Joseph Hammana for a Systems Programming class at Cal Poly SLO.

example run:

% ls
0  . 
0  .. 
1  357 

% cd 357
% ls

1  . 
0  .. 
2  assignment2 
3  final 
6  assignment3.tex 

% mkdir newDirectory
% cd newDirectory
% ls

7  . 
1  .. 

% touch newFile
% ls

7  . 
1  .. 
8  newFile 

% cd ..
% cd ..
% ls

0  . 
0  .. 
1  357 

% exit

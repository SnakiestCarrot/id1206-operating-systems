To run the program make sure you are in the same directory as this readme.txt and question7.c

To compile the code, in a terminal run:
make

Then run it with:
./question7 <number of pages> <O or 1 for normal page or huge page size>

To analyze the time and page faults run it with:
 /usr/bin/time -v ./question7 <number of pages> <option 0/1>

For example:
 /usr/bin/time -v ./question7 4096 1



In order to run this program, type "make" to compile the program, then ./pthread_sum {# of threads} to run it. 

Running "make clean" will remove the executable for you.

This program will create a sum of one million random floats between 0 and 1, and use a serial and parallel approach. The serial approach takes sums the matrix by using a for loop to iterate through every element in the array and add it to a sum variable. The parallel approach distributes the work by assigning each thread a portion of the array, and adds it to a shared array storing each thread's local sum, before finally the master thread sums all those local sums together. 

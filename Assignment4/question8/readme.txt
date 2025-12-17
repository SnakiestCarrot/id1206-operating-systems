
To generate the txt file simply run:

./generate_file.sh

This will create a file named "file_to_map.txt" with a size of 
exactly 1 MiB (1,048,576 bytes) in the current directory. If 
you want to specify a different filename, you can provide it 
as an argument to the script like this:

./generate_file.sh myfile.txt

The script will delete a file with the same filename (if it exists) 
and replace it.

To compile the C code just run:
make

This will compile the question8.c file and create an executable.
To run the compiled program, use:
./question8
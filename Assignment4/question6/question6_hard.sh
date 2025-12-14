#!/bin/bash

# create original file
echo "This is the starting text for the test with a hard link." > file_origin2.txt
echo "File created at file_origin2.txt."

# make hard link
echo -e "\nCreating link..."
# ln [TARGET] [LINK_NAME]
ln file_origin2.txt file_hard.txt
ls -l file_hard.txt

# check inode numbers
echo -e "\nComparing Inode numbers..."
# ls -li displays the inode number (first column)
ls -li file_origin2.txt file_hard.txt

echo -e "\nEditing file_hard.txt..."
echo "New Data: This line was added via the hard link." >> file_hard.txt

echo -e "\nChecking content of 'file_origin2.txt':"
echo "----------------------------------------------------"
cat file_origin2.txt
echo "----------------------------------------------------"
echo -e "\nChecking inodes again to see if they changed:"
ls -li file_origin2.txt file_hard.txt

# --- Question (h): Delete Original File ---
echo -e "\nDeleting 'file_origin2.txt'...\n"
rm file_origin2.txt

echo "Attempting to read 'file_hard.txt':"
echo "----------------------------------------------------"
if cat file_hard.txt; then
    echo "----------------------------------------------------"
    echo "SUCCESS: The file content is still accessible."
else
    echo "FAILURE: Could not read file."
fi

rm file_hard.txt
#!/bin/bash

# create original file
echo "Hello! This is some random text in the original file." > file_original.txt
echo -e "File created at file_original.txt.\n"

echo -e "Content of file_original.txt:"
cat file_original.txt

echo -e "\nCreating soft link at file_soft.txt -> file_original.txt..."
# ln -s [TARGET] [LINK_NAME]
ln -s file_original.txt file_soft.txt
ls -l file_soft.txt

# check inode numbers
# the -i flag in ls shows the inode number
ls -li file_original.txt file_soft.txt

# edit via soft link
echo -e "\nEditing file via soft link..."
echo "This line was appended by editing the soft link." >> file_soft.txt

echo -e "\nchecking content of 'file_original.txt' after edit..."
echo "-------------------------------------------------------------"
cat file_original.txt
echo "-------------------------------------------------------------"

# delete original file
rm file_original.txt

echo -e "\nattempting to read 'file_soft.txt' after deletion..."
if cat file_soft.txt 2>/dev/null; then
    echo "SUCCESS: Could read file after original was deleted!"
else
    echo "FAILURE: Could not read file after original was deleted!"
fi

# clean up directory
rm file_soft.txt
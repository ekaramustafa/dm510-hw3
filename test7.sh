#!/bin/bash

# Test 7: Test that inserting the maximum number of inodes in the filesystem.
# Check if trying to insert one above the max gives an error 

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'
MAGENTA='\e[35m'

cd ~/dm510fs-mountpoint/
# Clear current directory
clear_ls=$(ls)
rm -rf * 
clear_ls=$(ls)
expected_ls_result="file1
file10
file11
file12
file13
file14
file15
file2
file3
file4
file5
file6
file7
file8
file9" 

# Create files
for i in {1..15}
do
   touch "file$i"
done

echo "==================================================="
echo "Test 7: max inodes test" 
echo "Created 15 files"
echo "Executing ls command"
echo "The ls output"
echo -e "${YELLOW}$(ls)${RESET_COLOR}\n"
echo "Expected ls output"
echo -e "${YELLOW}$expected_ls_result${RESET_COLOR}"

# Compare ls output with the expected string
if [ "$(ls)" = "$expected_ls_result" ]; then
    echo -e "${GREEN}Test 7 ls Success${RESET_COLOR}"
else
    echo -e "${RED}Test 7 ls Fail${RESET_COLOR}"
fi

expected_touch_result="touch: cannot touch 'file16': No space left on device"
create_output=$(touch file16 2>&1)
echo
echo "Trying to insert another inode with touch command"
echo "The touch output"
echo -e "${YELLOW}$create_output${RESET_COLOR}\n"
echo "Expected touch output"
echo -e "${YELLOW}$expected_touch_result${RESET_COLOR}"

# Compare ls output with the expected string
if [ "$create_output" = "$expected_touch_result" ]; then
    echo -e "${GREEN}Test 7 touch Success${RESET_COLOR}"
else
    echo -e "${RED}Test 7 touch Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
#!/bin/bash

# Test 3: Test the removal of all files in directory

# Define colors
GREEN='\e[32m'
RED='\e[31m'
YELLOW='\e[33m'
RESET_COLOR='\e[0m'

cd ~/dm510fs-mountpoint/
# Clear current directory
rm -rf *
# Create a file
touch test.txt
# Create a directory
mkdir p
echo "==================================================="
echo "Test 3: removal of all files test" 
echo -e "Before removing ls command is executed"
echo -e "${YELLOW}$(ls)${RESET_COLOR}"
echo
# Check if there are files in the directory
if [ "$(ls -A)" ]; then
    # Remove all files in the directory
    rm -rf *
    
    # Check if files are removed successfully
    if [ "$(ls -A)" ]; then
        echo -e "${RED}Failed to remove all files.${RESET_COLOR}"
        echo -e "${RED}Test 3 Failed${RESET_COLOR}"
    else
        echo -e "${GREEN}All files removed successfully.${RESET_COLOR}"
        echo -e "After removing ls command is executed"
        echo -e "${YELLOW}$(ls)${RESET_COLOR}"
        echo -e "${GREEN}Test 3 Success${RESET_COLOR}"
    fi
else
    echo -e "${RED}No files found in the directory.${RESET_COLOR}"
    echo -e "${RED}Test 3 Failed${RESET_COLOR}"
fi
echo "==================================================="
echo

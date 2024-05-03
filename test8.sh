#!/bin/bash

# Test 8: Test that when trying to put a really long name to a file it gives an error

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'
MAGENTA='\e[35m'

cd ~/dm510fs-mountpoint/
# Clear current directory
rm -rf * 
clear_ls=$(ls)
expected_result="touch: cannot touch 'abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz': File name too long"
create_file_result=$(touch abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz 2>&1)

echo "==================================================="
echo "Test 8: max characters in name test" 
echo "Trying to create file with really long name"
echo "The output"
echo -e "${YELLOW}$create_file_result${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_result${RESET_COLOR}"

# Compare ls output with the expected string
if [ "$create_file_result" = "$expected_result" ]; then
    echo -e "${GREEN}Test 8 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 8 Fail${RESET_COLOR}"
fi

echo "==================================================="
echo
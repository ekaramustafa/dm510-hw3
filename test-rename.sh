#!/bin/bash

# Test: Rename test

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'

cd ~/dm510fs-mountpoint/
#Clear current directory
rm -rf *  
mkdir -p file1/file2
mkdir -p file1/file2/file3
mkdir -p file1/file4
touch file1/alp.txt
mv file1 file128
ls_output=$(ls)
expected_string="file128"

echo "==================================================="
echo "Test 7: rename test part-1" 
echo "Executing ls command"
echo "The output"
echo -e "${YELLOW}$(ls)${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_string${RESET_COLOR}"


if [ "$(ls)" = "$expected_string" ]; then
    echo -e "${GREEN}Test Part 1 Success${RESET_COLOR}"
else
    echo -e "${RED}Test Part 1 Fail${RESET_COLOR}"
fi

echo
echo "==================="
echo

cd file128
ls_output2=$(ls)
expected_string2="alp.txt
file2
file4"

echo "Test: rename test part-2" 
echo "Executing cd and ls command for child dirs"
echo "The output"
echo -e "${YELLOW}$ls_output2${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_string2${RESET_COLOR}"


if [ "$ls_output2" = "$expected_string2" ]; then
    echo -e "${GREEN}Test Part 2 Success${RESET_COLOR}"
else
    echo -e "${RED}Test Part 2 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
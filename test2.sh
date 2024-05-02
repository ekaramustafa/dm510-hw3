#!/bin/bash

# Test 2: Test the creation of a file with 'touch' command

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'
MAGENTA='\e[35m'

rm -rf * #remove the directory for the test2
expected_string="file.txt" 

touch file.txt
ls_output=$(ls)

echo "==================================================="
echo "Test 2 touch Test" 
echo "Executing touch file.txt command"
echo -e "The output: ${YELLOW}$ls_output${RESET_COLOR}"
echo -e "Expected output: ${YELLOW}$expected_string${RESET_COLOR}"

# Compare ls output with the expected string
if [ "$ls_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 2 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 2 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
#!/bin/bash

# Test 1: Test reading the current directory with 'ls' command

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'

#remove the directory for test1
rm -rf *
mkdir a
expected_string="a" 
ls_output=$(ls)

echo "==================================================="
echo "Test 1 ls Test" 
echo "Executing ls command"
echo -e "The output: ${YELLOW}$ls_output${RESET_COLOR}"
echo -e "Expected output: ${YELLOW}$expected_string${RESET_COLOR}"

# Compare ls output with the expected string
if [ "$ls_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 1 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 1 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
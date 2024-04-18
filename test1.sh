#!/bin/bash

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'

expected_string="a" 

ls_output=$(ls)

echo "================================"
echo "Test 1"
echo "The output: $ls_output"
echo "Expected output: $expected_string"

# Compare ls output with the expected string
if [ "$ls_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 1 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 1 Fail${RESET_COLOR}"
fi
echo "================================"
echo
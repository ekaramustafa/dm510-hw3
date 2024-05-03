#!/bin/bash

# Test 6: Write to a new file, then write again and read from it

# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'

cd ~/dm510fs-mountpoint/
#Clear current directory
rm -rf *  
mkdir file1
touch file1/alp.txt
cd file1
echo "Relax and Wololo" > alp.txt
echo "Oduncu, tamamdir" >> alp.txt

cat_output=$(cat alp.txt)
expected_string="Relax and Wololo
Oduncu, tamamdir"

echo "==================================================="
echo "Test 6: write&read second test" 
echo "Executing ls command"
echo "The output"
echo -e "${YELLOW}$cat_output${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_string${RESET_COLOR}"

if [ "$cat_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 6 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 6 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
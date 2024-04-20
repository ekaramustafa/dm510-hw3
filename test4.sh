# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'
MAGENTA='\e[35m'

rm -rf * #remove the directory for the test4
expected_string="alp.txt
file2" 
mkdir -p file1/file2
touch file1/alp.txt
cd file1
ls_output=$(ls)
echo "==================================================="
echo "Test 4 nested mkdir&ls Test" 
echo "Executing ls command"
echo "The output"
echo -e "${YELLOW}$(ls)${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_string${RESET_COLOR}"


# Compare ls output with the expected string
if [ "$ls_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 4 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 4 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
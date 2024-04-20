# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'
MAGENTA='\e[35m'

rm -rf * #remove the directory for the test4 
mkdir -p file1/file2
touch file1/alp.txt
cd file1
echo "Relax and Wololo" > alp.txt

cat_output=$(cat alp.txt)
expected_string="Relax and Wololo"

echo "==================================================="
echo "Test 5 write&read Test" 
echo "Executing cat alp.txt command"
echo "The output"
echo -e "${YELLOW}$cat_output${RESET_COLOR}\n"
echo "Expected output"
echo -e "${YELLOW}$expected_string${RESET_COLOR}"


if [ "$cat_output" = "$expected_string" ]; then
    echo -e "${GREEN}Test 5 Success${RESET_COLOR}"
else
    echo -e "${RED}Test 5 Fail${RESET_COLOR}"
fi
echo "==================================================="
echo
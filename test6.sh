# Define colors
GREEN='\e[32m'
RED='\e[31m'
RESET_COLOR='\e[0m'
YELLOW='\e[33m'

rm -rf * #remove the directory for the test4 
mkdir -p file1/file2
touch file1/alp.txt
cd file1
echo "Relax and Wololo" > alp.txt
echo "Oduncu, tamamdir" >> alp.txt

cat_output=$(cat alp.txt)
expected_string="Relax and Wololo
Oduncu, tamamdir"

echo "==================================================="
echo "Test 6 write&read Second Test" 
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
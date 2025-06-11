#!/bin/bash

cd "$(dirname "${BASH_SOURCE[0]}")"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

total_tests=0
passed_tests=0
failed_tests=0

declare -a failed_files
declare -a error_messages

declare -a should_fail_patterns=(
    "*/fail/*"
)

should_fail() {
    local file_path="$1"
    for pattern in "${should_fail_patterns[@]}"; do
        if [[ $file_path == $pattern ]]; then
            return 0
        fi
    done
    return 1
}

echo "Running tests..."
echo "==================="

while IFS= read -r -d '' file; do
    ((total_tests++))
    
    rel_path="${file#./}"
    
    printf "Testing %-50s " "$rel_path"
    
    if output=$(../probescript run "$file" 2>&1); then
        exit_code=0
    else
        exit_code=1
    fi
    
    if should_fail "$rel_path"; then
        if [ $exit_code -eq 1 ]; then
            echo -e "${GREEN}✓ FAIL (expected)${NC}"
            ((passed_tests++))
        else
            echo -e "${RED}✗ PASS (should fail)${NC}"
            ((failed_tests++))
            failed_files+=("$rel_path")
            error_messages+=("Expected this test to fail, but it passed")
        fi
    else
        if [ $exit_code -eq 0 ]; then
            echo -e "${GREEN}✓ PASS${NC}"
            ((passed_tests++))
        else
            echo -e "${RED}✗ FAIL${NC}"
            ((failed_tests++))
            failed_files+=("$rel_path")
            error_messages+=("$output")
        fi
    fi
    
done < <(find . -name "*.prb" -type f -print0)

echo
echo "==================="
echo "   Test Summary"
echo "==================="
echo -e "Total tests: ${YELLOW}$total_tests${NC}"
echo -e "Passed:      ${GREEN}$passed_tests${NC}"
echo -e "Failed:      ${RED}$failed_tests${NC}"

echo

if [ $failed_tests -gt 0 ]; then
    echo
    echo "❌ Failed Tests:"
    echo "================"
    
    for i in "${!failed_files[@]}"; do
        echo
        echo -e "${RED}🔸 ${failed_files[i]}${NC}"
        echo "   Error message:"
        echo "${error_messages[i]}" | sed 's/^/   │ /'
    done
    
    echo
    exit 1
else
    echo
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
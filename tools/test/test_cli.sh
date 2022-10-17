#!/bin/bash
#
# test_cli.sh
#
# The MIT License
#
# Copyright (c) 2022 Omics Data Automation, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Return Codes
OK=0
ERR=1
ABORT=134

# Check script inputs & print usage if the user has not provided required args
if [[ $# -lt 1 ]]; then
  echo "Usage: ./test_cli.sh <test_files_directory> [omicsds_binary_directory]"
  echo  "    <test_files_directory> directory containing files to use for testing."
  echo  "    [omicsds_binary_directory] optional path to omicsds binary to use."
  exit $ERR
fi

# Variable for directory containing test files
TEST_FILES_DIR=$(cd $1 && pwd)

# Add binary directory to PATH, if the user has provided it
if [[ -n $2 ]]; then
  cd $2 && PATH=$(pwd):$PATH
fi

# Setup temporary workspace directory to write test results to
TEMP_DIR=$(mktemp -d -t omicds_test_cli-XXXXXXXXXX)
if [[ $? -ne 0 ]]; then
  echo "Failed to create temporary directory"
  exit $ERR
fi
WORKSPACE_DIR=$TEMP_DIR/workspace

# Remove the temporary directory where test files are generated
cleanup() {
  rm -rf $TEMP_DIR
}

# Exits the test script with specified code, optionally displaying a message
#    $1 - return code
#    $2 - optional message to be displayed on exit
die() {
  cleanup
  if [[ $# -eq 2 ]]; then
    echo $2
  fi
  exit $1
}

# Run the given command, checking that the return code is as expected. Returns
# the file containing the contents of stdout.
#    $1 - the command to be run
#    $2 - expected return code of the command
#    $3 - working directory for running the command. Optional defaults to pwd.
run_command() {
  echo "" > $TEMP_DIR/test.stdout
  echo "" > $TEMP_DIR/test.stderr
  RUN_DIR=${3:-$(pwd)}
  cd $RUN_DIR
  $1 > $TEMP_DIR/test.stdout 2> $TEMP_DIR/test.stderr
  if [[ $? -ne $2 ]]; then
    echo "Test stdout"
    cat $TEMP_DIR/test.stdout
    echo "Test stderr"
    cat $TEMP_DIR/test.stderr
    die $ERR "Command $1 did not return code $2."
  fi
  cd - > /dev/null
  echo $TEMP_DIR/test.stdout
}

# Queries the specified array for a matrix, then checks that the resulting matrix file has the expected dimensions.
#    $1 - name of the array to generate matrix from
#    $2 - number of rows
#    $3 - number of columns
check_matrix() {
  matrix_file=$(run_command "omicsds query -w ${WORKSPACE_DIR} -a $1 -x" $OK $TEMP_DIR)
  row_count=$(wc -l < $matrix_file)
  echo $row_count
  if [[ $row_count -ne $2 ]]; then
    cat $matrix_file
    die $ERR "Incorrect number of rows. Expected $2 found $row_count."
  fi
  column_count=$(head -1 $matrix_file | wc -w)
  if [[ $column_count -ne $3 ]]; then
    cat $matrix_file
    die $ERR "Incorrect number of columns. Expected $3 found $column_count."
  fi
}

# Runs the passed in command and checks that the expected regex string appears in the output.
#    $1 - command to check
#    $2 - string to check for in output
check_output() {
  output_file=$(run_command "$1" $OK $TEMP_DIR)
  grep -qE -m1 "$2" "$output_file"
  if [[ $? -ne 0 ]]; then
    cat $output_file
    die $ERR "Failed to find $2 in file $outputfile"
  fi
}

# Counts the number of files or directories in a directory and returns an error if the count is not correct.
# Optionally, a name filter can be provided to be applied to the list of files
#    $1 - directory to count
#    $2 - expected number of files
#    $3 - optional filter on the name of files to count
count_files() {
  expression=${3:-"*"}
  num_files=$(find $1 -maxdepth 1 \! -path $1 \( -type f -o -type d \) -name "$expression" | wc -l)
  if [[ $num_files -ne $2 ]]; then
    ls -la $1
    die $ERR "Wrong number of files in $1. Expected $2 found $num_files."
  fi
}

# Example 1: Ingest and export 2 sam files
run_command "omicsds import -m human_g1k_v37.fasta.fai -w ${WORKSPACE_DIR} -a sam_array -l sam_list -r -s sam_map" $OK $TEST_FILES_DIR
run_command "omicsds query -w ${WORKSPACE_DIR} -a sam_array --export-sam" $OK $TEMP_DIR
count_files $TEMP_DIR 2 "sam_output*"

# Example 2: Ingest two bed files with interval-level ingestion
run_command "omicsds import -m human_g1k_v37.fasta.fai -w ${WORKSPACE_DIR} -a bed_array -i -l small_list -s small_map" $OK $TEST_FILES_DIR
count_files ${WORKSPACE_DIR}/bed_array 4

# Example 3: Ingest small matrix file (unsorted matrix)
run_command "omicsds import -w ${WORKSPACE_DIR} -a matrix_array -f -l small_matrix_list -s small_map" $OK $TEST_FILES_DIR
check_matrix "matrix_array" 3 305

# Example 4: Ingest small matrix file (sorted matrix)
run_command "omicsds import -w ${WORKSPACE_DIR} -a sorted_matrix_array -f -l small_matrix_sorted_list -s small_map" $OK $TEST_FILES_DIR
check_matrix "sorted_matrix_array" 3 305

# Example 5: Preconfigure workspace for import
run_command "omicsds configure -w ${WORKSPACE_DIR} -f -l small_matrix_sorted_list -s small_map" $OK $TEST_FILES_DIR
run_command "omicsds import -w ${WORKSPACE_DIR} -a configured_matrix_array " $OK $TEST_FILES_DIR
check_matrix "configured_matrix_array" 3 305

# Example 6: Consolidate workspace for import
run_command "omicsds import -w ${WORKSPACE_DIR} -a consolidate_matrix_array -f -l small_matrix_list -s small_map -c" $OK $TEST_FILES_DIR
check_matrix "consolidate_matrix_array" 3 305
count_files ${WORKSPACE_DIR}/consolidate_matrix_array 5

# Check help text
check_output "omicsds" "Usage: omicsds <command> <arguments>"
for command in "configure" "import" "query"; do
  check_output "omicsds $command" "Usage: omicsds $command \[options\]"
done

# Import non-existent file list
run_command "omicsds import -w ${WORKSPACE_DIR} -a bad_array -f -l missing_list -s small_map -c" $ABORT $TEST_FILES_DIR

# Test consolidate after import
run_command "omicsds import -w ${WORKSPACE_DIR} -a post_consolidate_matrix_array -f -l small_matrix_list -s small_map" $OK $TEST_FILES_DIR
check_matrix "post_consolidate_matrix_array" 3 305
count_files ${WORKSPACE_DIR}/post_consolidate_matrix_array 6
run_command "omicsds consolidate -w ${WORKSPACE_DIR} -a post_consolidate_matrix_array" $OK $TEST_FILES_DIR
check_matrix "post_consolidate_matrix_array" 3 305
count_files ${WORKSPACE_DIR}/post_consolidate_matrix_array 5

die $OK "All tests passed!"

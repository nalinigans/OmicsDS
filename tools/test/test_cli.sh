#!/bin/bash
set -e
OMICSDS_ROOT_DIR=${OMICSDS_ROOT_DIR:-$(pwd)/../..}
OMICSDS_BUILD_DIR=${OMICSDS_BUILD_DIR:-${OMICSDS_ROOT_DIR}/build}
WORKSPACE_DIR=${WORKSPACE:-$(mktemp -d)}

pushd ${OMICSDS_BUILD_DIR}/tools/main
OMICSDS_BIN_PATH=$(pwd)
popd

pushd ${OMICSDS_ROOT_DIR}/src/test/inputs/OmicsDSTests
# Example 1: Ingest and Export two sam files
${OMICSDS_BIN_PATH}/omicsds import -m human_g1k_v37.fasta.fai -w ${WORKSPACE_DIR}/workspace -a sam_array -l sam_list -r -s sam_map
${OMICSDS_BIN_PATH}/omicsds query -w ${WORKSPACE_DIR}/workspace -a sam_array --export-sam > ${WORKSPACE_DIR}/sam.query
if [[ $? != 0 ]]; then
    echo "Example 1 failed"
    exit 1
fi

# Example 2: Ingest two bed files with interval-level ingestion
${OMICSDS_BIN_PATH}/omicsds import -m human_g1k_v37.fasta.fai -w ${WORKSPACE_DIR}/workspace -a bed_array -i -l small_list -s small_map > ${WORKSPACE_DIR}/bed.import
if [[ $? != 0  ]]; then
    echo "Example 2 failed"
    exit 1
fi

# Example 3: Ingest small matrix file (unsorted matrix)
${OMICSDS_BIN_PATH}/omicsds import -w ${WORKSPACE_DIR}/workspace -a matrix_array -f -l small_matrix_list -s small_map
${OMICSDS_BIN_PATH}/omicsds query -w ${WORKSPACE_DIR}/workspace -a matrix_array -x > ${WORKSPACE_DIR}/matrix.output
if [[ $(sed -n '=' ${WORKSPACE_DIR}/matrix.output | wc -l | xargs) != 3 || $(head -1 ${WORKSPACE_DIR}/matrix.output | wc -w | xargs) != 305 ]]; then
    echo $(sed -n '=' ${WORKSPACE_DIR}/matrix.output | wc -l)
    echo $(head -1 ${WORKSPACE_DIR}/matrix.output | wc -w)
    echo "Example 3 failed"
    exit 1
fi

# Example 4: Ingest small matrix file (sorted matrix)
${OMICSDS_BIN_PATH}/omicsds import -w ${WORKSPACE_DIR}/workspace -a sorted_matrix_array -f -l small_matrix_sorted_list -s small_map
${OMICSDS_BIN_PATH}/omicsds query -w ${WORKSPACE_DIR}/workspace -a sorted_matrix_array -x > ${WORKSPACE_DIR}/matrix.output
if [[ $(sed -n '=' ${WORKSPACE_DIR}/matrix.output | wc -l | xargs) != 3 || $(head -1 ${WORKSPACE_DIR}/matrix.output | wc -w | xargs) != 305 ]]; then
    echo "Example 4 failed"
    exit 1
fi

# Example 5: Preconfigure workspace for import
${OMICSDS_BIN_PATH}/omicsds configure -w ${WORKSPACE_DIR}/workspace -f -l small_matrix_sorted_list -s small_map
${OMICSDS_BIN_PATH}/omicsds import -w ${WORKSPACE_DIR}/workspace -a configured_matrix_array 
${OMICSDS_BIN_PATH}/omicsds query -w ${WORKSPACE_DIR}/workspace -a configured_matrix_array -x > ${WORKSPACE_DIR}/matrix.output
if [[ $(sed -n '=' ${WORKSPACE_DIR}/matrix.output | wc -l | xargs) != 3 || $(head -1 ${WORKSPACE_DIR}/matrix.output | wc -w | xargs) != 305 ]]; then
    echo "Example 5 failed"
    exit 1
fi

# Example 6: Consolidate workspace for import
${OMICSDS_BIN_PATH}/omicsds import -w ${WORKSPACE_DIR}/workspace -a consolidate_matrix_array -f -l small_matrix_list -s small_map -c
FILE_COUNT=0
for file in ${WORKSPACE_DIR}/workspace/consolidate_matrix_array/*; do
    FILE_COUNT=$((FILE_COUNT+1))
done
if [[ $FILE_COUNT != 4 ]]; then
    echo "Example 6 failed"
    exit 1
fi

# Cleanup 
rm -r ${WORKSPACE_DIR}/*
popd

#!/bin/bash

./cleanup

get_omicsds_home() {
  if [[ -z $OMICSDS_HOME ]]; then
     pushd ../../install
     export OMICSDS_HOME=$(pwd -P)
     popd
  fi
}

get_omicsds_home
echo "Using OMICSDS_HOME=$OMICSDS_HOME"

R -e 'library(Rcpp); compileAttributes(".")'
R CMD build . &&
R CMD INSTALL --preclean --configure-args="--with-omicsds=${OMICSDS_HOME}"  omicsds_0.0.1.tar.gz &&
#R CMD check --no-manual --install-args="--configure-args='--with-omicsds=${OMICDS_HOME}'" omicsds_0.0.1.tar.gz # &&
R -e 'library(testthat); test_local()'

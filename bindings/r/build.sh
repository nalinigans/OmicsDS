#!/bin/bash

./cleanup

get_omicsds_home() {
  if [[ -z $OMICSDS_HOME ]]; then
    pushd ../../install
    export OMICSDS_HOME=$(pwd -P)
    popd
  fi
}

get_omicsds_repository() {
  if [[ -z $OMICSDS_REPOSITORY ]]; then
    pushd ../..
    export OMICSDS_REPOSITORY=$(pwd -P)
    popd
  fi
}

get_omicsds_home
echo "Using OMICSDS_HOME=$OMICSDS_HOME"

get_omicsds_repository
echo "Using OMICSDS_REPOSITORY=$OMICSDS_REPOSITORY"

R -e 'library(Rcpp); compileAttributes(".")'
#R -e 'library(roxygen2); roxygen2::roxygenize(roclets="rd")'
#R -e 'library(usethis); usethis::use_vignette("omicsds"
R CMD build . &&
R CMD INSTALL --preclean --configure-args="--with-omicsds=${OMICSDS_HOME}"  omicsds_0.0.1.tar.gz &&
R CMD check --no-manual --no-build-vignettes --no-vignettes  --install-args="--configure-args='--with-omicsds=${OMICSDS_HOME}'" omicsds_0.0.1.tar.gz &&
R CMD check --no-manual --install-args="--configure-args='--with-omicsds=${OMICSDS_HOME}'" omicsds_0.0.1.tar.gz &&
R -e 'library(testthat); test_local()'

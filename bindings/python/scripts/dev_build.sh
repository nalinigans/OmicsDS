# Copyright (c) 2024 dātma, inc™
#
# Proprietary and Confidential:
# Unauthorized copying of this file by any medium is strictly prohibited.
#
#!/bin/bash

python -m build -C="--build-option=--cythonize" -C="--build-option=--inplace" &&
if [[ $(uname) == "Linux" ]]; then export LD_LIBRARY_PATH=$OMICSDS_HOME/lib:$LD_LIBRARY_PATH; fi &&
pip uninstall -y omicsds && pip install dist/omicsds-0.0.1-cp3*whl && pytest -s

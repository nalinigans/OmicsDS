# Copyright (c) 2024 dātma, inc™
#
# Proprietary and Confidential:
# Unauthorized copying of this file by any medium is strictly prohibited.
#
#!/bin/bash

python -m build -C="--build-option=--cythonize" -C="--build-option=--inplace" &&
pip uninstall -y omicsds && pip install dist/omicsds-0.0.1-cp311-cp311-macosx_14_0_arm64.whl && pytest -s

#
# conftest.py
#
# The MIT License
#
# Copyright (c) 2023 Omics Data Automation, Inc.
# Copyright (C) 2023 dātma, inc™
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
import pytest
import os
import pathlib
import omicsds.api


@pytest.fixture(scope="session")
def test_inputs_dir():
    path_root = pathlib.Path(__file__).parent.parent.parent.parent.resolve().as_posix()
    root = pathlib.Path(os.environ.get("OMICSDS_REPOSITORY", path_root))
    return root.joinpath("src/test/inputs").as_posix()


@pytest.fixture(scope="session")
def workspace(test_inputs_dir):
    return os.path.join(test_inputs_dir, "feature-level-ws")


@pytest.fixture(scope="session")
def array():
    return "array"


@pytest.fixture(scope="function")
def omicsds_handle(workspace, array):
    handle = omicsds.api.connect(workspace, array)
    yield handle
    omicsds.api.disconnect(handle)

#
# test_bindings.py
#
# The MIT License
#
# Copyright (c) 2023 Omics Data Automation, Inc.
# Copyright (c) 2023 dātma, inc™
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
import pandas as pd
import numpy as np
import omicsds.api
import os, sys
import orjson


def test_connect(omicsds_handle):
    assert omicsds_handle is not None


def test_restricted_query(omicsds_handle):
    features = ["ENSG00000138190", "ENSG00000243485"]
    df = omicsds.api.query_features(omicsds_handle, features, (0, 2))
    expected_df = pd.DataFrame(
        [[1488.0, 177.0, 405.0], [828.0, 153.0, 2301.0]], index=features, dtype=np.float32
    )
    assert expected_df.equals(df)
    assert (expected_df.index == df.index).all()
    assert (expected_df.columns == df.columns).all()

    json = omicsds.api.query_features(
        omicsds_handle, features, (0, 2), output_mode=omicsds.api.OutputMode.JSON_BY_SAMPLE
    )
    expected_json = '{"samples":[0,1,2],"ENSG00000138190":[1488.0,177.0,405.0],"ENSG00000243485":[828.0,153.0,2301.0]}'
    assert json == expected_json

    json = omicsds.api.query_features(
        omicsds_handle, features, (0, 2), output_mode=omicsds.api.OutputMode.JSON_BY_FEATURE
    )
    expected_json = '{"features":["ENSG00000138190","ENSG00000243485"],"0":[1488.0,828.0],"1":[177.0,153.0],"2":[405.0,2301.0]}'
    assert json == expected_json


def test_full_query(omicsds_handle):
    features = ["ENSG00000138190", "ENSG00000243485"]
    df = omicsds.api.query_features(omicsds_handle)
    assert (df.index == features).all()
    assert (df.columns == range(304)).all()
    assert df[10]["ENSG00000138190"] == 0.0

    INPUTS_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "inputs")
    json = omicsds.api.query_features(
        omicsds_handle, output_mode=omicsds.api.OutputMode.JSON_BY_SAMPLE
    )
    with open(os.path.join(INPUTS_DIR, "omicsds_output_by_sample.json"), "r") as j:
        expected_json = j.read()
    assert len(json) == len(expected_json)
    assert json == expected_json

    json = omicsds.api.query_features(
        omicsds_handle, output_mode=omicsds.api.OutputMode.JSON_BY_FEATURE
    )
    with open(os.path.join(INPUTS_DIR, "omicsds_output_by_feature.json"), "r") as j:
        expected_json = j.read()
    assert len(json) == len(expected_json)
    assert json == expected_json


def test_no_workspace():
    with pytest.raises(Exception):
        handle = omicsds.api.connect("/no-workspace", "array")

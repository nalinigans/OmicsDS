# distutils: language = c++
# cython: language_level=3
#
# api.pyx
#
# The MIT License
#
# Copyright (c) 2023 Omics Data Automation, Inc.
 * Copyright (c) 2023 dātma, inc™
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

from typing import Optional

from libc.stdint cimport INT64_MAX

import numpy as np

cimport numpy as np

np.import_array()

import pandas as pd


def version() -> str:
    return OmicsDS.version().decode(encoding="ascii")


def connect(workspace: str, array: str) -> int:
    return OmicsDS.connect(workspace.encode(encoding="ascii"), array.encode(encoding="ascii"))


def disconnect(handle: int) -> None:
    OmicsDS.disconnect(handle)


def query_features(
    handle: int,
    features: Optional[list[str]] = None,
    sample_range: Optional[tuple[int, int]] = None
) -> pd.DataFrame:
    cdef vector[string] feature_results
    cdef vector[uint64_t] sample_results
    cdef vector[float] score_results
    processor = new OmicsDSProcessor(&feature_results, &sample_results, &score_results)
    if features is None:
        features = []
    else:
        features = [f.encode(encoding="ascii") for f in features]
    if sample_range is None:
        sample_range = (0, INT64_MAX)
    OmicsDS.query_features(handle, features, sample_range, processor[0])

    cdef np.ndarray results = np.array(score_results, dtype=np.single, copy=False)
    results = results.reshape((len(feature_results), len(sample_results)))

    decoded_features = [feature.decode(encoding="ascii") for feature in feature_results]

    return pd.DataFrame(data=results, index=decoded_features, columns=sample_results)

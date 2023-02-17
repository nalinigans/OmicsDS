#
# api.pxd
#
# The MIT License
#
# Copyright (c) 2023 Omics Data Automation, Inc.
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
# distutils: language = c++
# cython: language_level=3

from libc.stdint cimport int64_t, uint8_t, uint64_t
from libcpp.functional cimport function
from libcpp.pair cimport pair
from libcpp.string cimport string
from libcpp.vector cimport vector


cdef extern from "omicsds_processor.h":
    cdef cppclass OmicsDSProcessor:
        OmicsDSProcessor(vector[string]* features, vector[uint64_t]* samples, vector[float]* scores)


cdef extern from "omicsds.h":
    ctypedef size_t OmicsDSHandle

    cdef cppclass OmicsDS:
        @staticmethod
        string version()

        @staticmethod
        OmicsDSHandle connect(const string& workspace, const string& array) except +

        @staticmethod
        void disconnect(OmicsDSHandle handle)

        @staticmethod
        void query_features(OmicsDSHandle handle, vector[string]& features,
                             pair[int64_t, int64_t]& sample_range,
                             OmicsDSProcessor proc) except +

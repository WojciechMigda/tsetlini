# distutils: language = c++

from libcpp.pair cimport pair
from libcpp.string cimport string

cdef extern from "tsetlin_status_code.hpp" namespace "Tsetlin":
    cdef enum StatusCode:
        S_OK = 0
        S_BAD_JSON
        S_BAD_LABELS
    ctypedef pair[StatusCode, string] status_message_t

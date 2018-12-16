# -*- coding: utf-8 -*-
# distutils: language = c++
#
# cython: wraparound  = False
# cython: boundscheck = False
# cython: cdivision   = True

import  numpy as np
cimport numpy as np
cimport tsetlin_tk.libtsetlin

from tsetlin_tk.either cimport Either

from tsetlin_tk.tsetlin_status_code cimport status_message_t
from tsetlin_tk.tsetlin_classifier_state cimport ClassifierState
from tsetlin_tk.tsetlin_types cimport aligned_vector_char, label_vector_type, label_type
from tsetlin_tk.tsetlin_state_json cimport to_json_string

from libcpp.string cimport string
from libcpp.vector cimport vector


cdef status_message_t raise_value_error(status_message_t && msg):
    raise ValueError(<bytes>msg.second)
    return msg


cdef extern from "<utility>" namespace "std" nogil:
    cdef Either[status_message_t, string] && move "std::move"(Either[status_message_t, string])



cdef vector[aligned_vector_char] X_as_vectors(np.ndarray X, bint is_sparse):
    cdef vector[aligned_vector_char] rv

    rv.resize(X.shape[0])

    for rit in range(X.shape[0]):
        rv[rit].resize(X.shape[1])

        if is_sparse:
            for cit in range(X.shape[1]):
                rv[rit][cit] = X[rit, cit]
        else:
            for cit in range(X.shape[1]):
                rv[rit][cit] = X[rit, cit]

    return rv


cdef label_vector_type y_as_vector(np.ndarray y, bint is_sparse):
    cdef label_vector_type rv

    rv.resize(y.shape[0])

    if is_sparse:
        for it in range(y.shape[0]):
            rv[it] = y[it]
    else:
        for it in range(y.shape[0]):
            rv[it] = y[it]

    return rv


cdef extern from "tsetlin_private.hpp":
    cdef Either[status_message_t, string] train_lambda """
[](std::string const & params, std::vector<Tsetlin::aligned_vector_char> const & X, Tsetlin::label_vector_type const & y, int n_epochs)
{
    return
    Tsetlin::make_params_from_json(params)
        .rightMap([](auto && params){ return Tsetlin::ClassifierState(params); })
        .rightFlatMap([&X, &y, n_epochs](auto && state)
        {
            auto status = Tsetlin::fit_impl(state, X, y, n_epochs);

            if (status.first == Tsetlin::S_OK)
            {
                std::string js_state = Tsetlin::to_json_string(state);

                return neither::Either<Tsetlin::status_message_t, std::string>::rightOf(js_state);
            }
            else
            {
                return neither::Either<Tsetlin::status_message_t, std::string>::leftOf(status);
            }
        })
        ;
}
"""(string params, vector[aligned_vector_char] X, label_vector_type y, int n_epochs)


cdef extern from "tsetlin_private.hpp":
    cdef Either[status_message_t, label_vector_type] predict_lambda """
[](std::string const & js_model, std::vector<Tsetlin::aligned_vector_char> const & X)
{
    Tsetlin::ClassifierState state(Tsetlin::params_t{});

    Tsetlin::from_json_string(state, js_model);

    return Tsetlin::predict_impl(state, X);
}
"""(string js_model, vector[aligned_vector_char] X)


cdef extern from *:
    cdef Either[string, string] reduce_status_message_to_string """
[](Tsetlin::status_message_t && msg)
{
    return neither::Either<std::string, std::string>::leftOf(std::string());
}
""" (msg)


cdef extern from *:
    cdef Either[label_vector_type, label_vector_type] reduce_status_message_to_label_vector """
[](Tsetlin::status_message_t && msg)
{
    return neither::Either<Tsetlin::label_vector_type, Tsetlin::label_vector_type>::leftOf(Tsetlin::label_vector_type());
}
""" (msg)



def classifier_fit(np.ndarray npX, bint X_is_sparse, np.ndarray npy, bint y_is_sparse, bytes js_params, int n_epochs):

    """
    Going with the most basic and general input preparation - deep copy X and y into c++ vectors
    """
    cdef label_vector_type y = y_as_vector(npy, y_is_sparse)
    cdef vector[aligned_vector_char] X = X_as_vectors(npX, X_is_sparse)

    cdef string js_state = \
        train_lambda(<string>js_params, X, y, n_epochs) \
            .leftMap(raise_value_error) \
            .leftFlatMap(reduce_status_message_to_string) \
            ._join[string]()

    return js_state


def classifier_predict(np.ndarray npX, bint X_is_sparse, bytes js_model):

    cdef vector[aligned_vector_char] X = X_as_vectors(npX, X_is_sparse)

    cdef label_vector_type labels = \
         predict_lambda(<string>js_model, X) \
            .leftMap(raise_value_error) \
            .leftFlatMap(reduce_status_message_to_label_vector) \
            ._join[label_vector_type]()

    cdef label_type * labels_p = labels.data()
    cdef label_type[::1] vec_view = <label_type[:labels.size()]>labels_p
    return np.array(vec_view)

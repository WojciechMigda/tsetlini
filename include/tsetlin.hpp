#pragma once

#include "neither/either.hpp"

#include "tsetlin_params.hpp"
#include "tsetlin_status_code.hpp"

#include "tsetlin_types.hpp"
#include "tsetlin_state.hpp"

#include <vector>

namespace Tsetlin
{

using namespace neither;

struct Classifier
{

//    void update(aligned_vector_char const & X, label_vector_type::value_type target_label);
//
//    void fit_batch(std::vector<aligned_vector_char> const & X, label_vector_type const & y);

    status_message_t
    fit(std::vector<aligned_vector_char> const & X, label_vector_type const & y, int epochs = 100)
        __attribute__((warn_unused_result));

    Either<status_message_t, real_type>
    evaluate(std::vector<aligned_vector_char> const & X, label_vector_type const & y) const
        __attribute__((warn_unused_result));

    Either<status_message_t, label_type>
    predict(aligned_vector_char const & sample) const
        __attribute__((warn_unused_result));
//
//    aligned_vector_int predict_raw(aligned_vector_char const & sample) const;
//
//    void predict_raw(aligned_vector_char const & sample, int * out_p) const;
//
    params_t read_params() const;

friend Either<status_message_t, Classifier> make_classifier(std::string const & json_params);


private:
    ClassifierState m_state;

    Classifier(params_t const & params);
    Classifier(params_t && params);
};

Either<status_message_t, Classifier> make_classifier(std::string const & json_params = "{}");


} // namespace Tsetlin

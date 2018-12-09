#pragma once

#include "tsetlin_params.hpp"

#include <variant>

namespace Tsetlin
{

namespace Params
{


inline
int n_jobs(params_t const & params)
{
    return std::get<int>(params.at("n_jobs"));
}


inline
seed_type random_state(params_t const & params)
{
    return std::get<seed_type>(params.at("random_state"));
}


inline
bool verbose(params_t const & params)
{
    return std::get<bool>(params.at("verbose"));
}


inline
int number_of_pos_neg_clauses_per_label(params_t const & params)
{
    return std::get<int>(params.at("number_of_pos_neg_clauses_per_label"));
}


inline
int number_of_labels(params_t const & params)
{
    return std::get<int>(params.at("number_of_labels"));
}


inline
int number_of_features(params_t const & params)
{
    return std::get<int>(params.at("number_of_features"));
}


inline
real_type s(params_t const & params)
{
    return std::get<real_type>(params.at("s"));
}


inline
int threshold(params_t const & params)
{
    return std::get<int>(params.at("threshold"));
}


inline
int boost_true_positive_feedback(params_t const & params)
{
    return std::get<int>(params.at("boost_true_positive_feedback"));
}


inline
int number_of_states(params_t const & params)
{
    return std::get<int>(params.at("number_of_states"));
}


inline
int number_of_clauses_per_label(params_t const & params)
{
    return 2 * number_of_pos_neg_clauses_per_label(params);
}


inline
int number_of_clauses(params_t const & params)
{
    return number_of_clauses_per_label(params) * number_of_labels(params);
}

} // namespace Params

} // namespace Tsetlin
#define LOG_MODULE "tsetlin-core"
#include "logger.hpp"

#include "tsetlin_state.hpp"
#include "tsetlin_types.hpp"
#include "params_companion.hpp"
#include "tsetlin_params.hpp"
#include "tsetlin_classifier_state_private.hpp"

#include <algorithm>
#include <iterator>
#include <thread>
#include <random>
#include <limits>


namespace std
{
// this is needed for std::variant comparison, probably illegal. TODO
static constexpr
bool operator==(nullopt_t const & lhs, nullopt_t const & rhs){ return true; }

}


namespace Tsetlin
{


void initialize_state(ClassifierState & state)
{
    auto & params = state.m_params;

    auto const verbose = Params::verbose(params);

    state.igen.init(Params::random_state(params));
    state.fgen.init(Params::random_state(params));

    LOG(info) << "number_of_labels: " << Params::number_of_labels(params) << '\n';
    LOG(info) << "number_of_clauses: " << Params::number_of_clauses(params) << '\n';
    LOG(info) << "number_of_features: " << Params::number_of_features(params) << '\n';
    LOG(info) << "s: " << Params::s(params) << '\n';
    LOG(info) << "number_of_states: " << Params::number_of_states(params) << '\n';
    LOG(info) << "threshold: " << Params::threshold(params) << '\n';
    LOG(info) << "counting_type: " << Params::counting_type(params) << '\n';
    LOG(info) << "n_jobs: " << Params::n_jobs(params) << '\n';
    LOG(info) << "random_state: " << Params::random_state(params) << '\n';

    // convenience reference variables
    auto & ta_state_v = state.ta_state;
    auto & igen = state.igen;

    auto const number_of_states = Params::number_of_states(params);
    auto const & counting_type = Params::counting_type(params);

    if (number_of_states <= std::numeric_limits<std::int8_t>::max()
        and ("auto" == counting_type or "int8" == counting_type))
    {
        LOG(trace) << "Selected int8 for ta_state\n";
        ta_state_v = std::vector<aligned_vector_int8>();
    }
    else if (number_of_states <= std::numeric_limits<std::int16_t>::max()
        and ("auto" == counting_type or "int8" == counting_type or "int16" == counting_type))
    {
        LOG(trace) << "Selected int16 for ta_state\n";
        ta_state_v = std::vector<aligned_vector_int16>();
    }
    else
    {
        LOG(trace) << "Selected int32 for ta_state\n";
        ta_state_v = std::vector<aligned_vector_int32>();
    }

    auto ta_state_gen = [&params, &igen](auto & ta_state)
    {
        using row_type = typename std::decay<decltype(ta_state)>::type::value_type;

        std::generate_n(std::back_inserter(ta_state), Params::number_of_clauses(params),
            [&params, &igen]()
            {
                row_type rv;

                std::generate_n(std::back_inserter(rv), Params::number_of_features(params) * 2,
                    [&params, &igen]()
                    {
                        return igen.next(-1, 0);
                    }
                );

                return rv;
            }
        );
    };

    std::visit(ta_state_gen, ta_state_v);

    reset_state_cache(state);
}


void reset_state_cache(ClassifierState & state)
{
    auto & cache = state.cache;
    auto & params = state.m_params;

    cache.clause_output.clear();
    cache.clause_output.resize(Params::number_of_clauses(params));
    cache.label_sum.clear();
    cache.label_sum.resize(Params::number_of_labels(params));
    cache.feedback_to_clauses.clear();
    cache.feedback_to_clauses.resize(Params::number_of_clauses(params));

    // initialize frand caches instances for use by all thread jobs
    cache.fcache.reserve(Params::n_jobs(params));

    for (auto it = 0; it < Params::n_jobs(params); ++it)
    {
        cache.fcache.emplace_back(2 * Params::number_of_features(params), state.igen.peek() + it);
    }
}


ClassifierState::ClassifierState(params_t const & params) :
    m_params(params)
{
}


bool ClassifierState::operator==(ClassifierState const & other) const
{
    if (this == &other)
    {
        return true;
    }
    else
    {
        return
            ta_state == other.ta_state
            and igen == other.igen
            and fgen == other.fgen
            and m_params == other.m_params
            and cache.feedback_to_clauses.size() == other.cache.feedback_to_clauses.size()
            and cache.clause_output.size() == other.cache.clause_output.size()
            and cache.label_sum.size() == other.cache.label_sum.size()
            and cache.fcache.size() == other.cache.fcache.size()
            ;
    }
}

} // namespace Tsetlin

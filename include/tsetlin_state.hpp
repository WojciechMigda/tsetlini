#pragma once

#include "frand_cache.hpp"
#include "mt.hpp"

#include "tsetlin_params.hpp"
#include "tsetlin_types.hpp"

#include <vector>

namespace Tsetlin
{


struct ClassifierState
{
    using frand_cache_type = frand_cache<FRNG, alignment>;
    using ta_state_v_type = std::variant<
        std::vector<aligned_vector_int32>
        , std::vector<aligned_vector_int16>
        , std::vector<aligned_vector_int8>
    >;

    struct Cache
    {
        feedback_vector_type feedback_to_clauses; // shape=(number of clauses)
        aligned_vector_char clause_output; // shape=(number of clauses)
        aligned_vector_int label_sum; // shape=(number of labels)

        std::vector<frand_cache_type> fcache;
    };

    params_t m_params;

    ta_state_v_type ta_state;

    mutable Cache cache;

    IRNG igen;
    FRNG fgen;

    explicit ClassifierState(params_t const & params);

    bool operator==(ClassifierState const & other) const;
};


} // namespace Tsetlin

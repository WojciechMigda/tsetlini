#include "tsetlini_algo_bitwise.hpp"
#include "tsetlini_strong_params.hpp"
#include "tsetlini_strong_params_private.hpp"
#include "tsetlini_types.hpp"
#include "estimator_state.hpp"

#include "strong_type/strong_type.hpp"
#include "rapidcheck.h"
#include "boost/ut.hpp"

#include <cstdlib>
#include <cstdint>
#include <random>
#include <algorithm>
#include <cmath>


using namespace boost::ut;


auto constexpr MAX_NUM_OF_FEATURES = 800;
auto constexpr MAX_NUM_OF_CLAUSE_OUTPUTS = 16;
auto constexpr MAX_NUM_OF_STATES = 1000;
auto constexpr MAX_THRESHOLD = 1024;

auto constexpr MIN_RESPONSE_ERROR = std::numeric_limits<strong::underlying_type_t<Tsetlini::response_error_t>>::min();
auto constexpr MAX_RESPONSE_ERROR = std::numeric_limits<strong::underlying_type_t<Tsetlini::response_error_t>>::max();

/*
 * `inRange` is exclusive on upper bound, so this is OK as max value for filling
 * weight test vectors.
 *
 *      weight = [0, MAX_WEIGHT)
 *
 * In real life scenario weight will never equal MAX_WEIGHT, because for
 * incrementation it is compared against `max_weight` after adding +1 to it.
 */
std::uint32_t constexpr MAX_WEIGHT = value_of(Tsetlini::MAX_WEIGHT_DEFAULT);
std::uint32_t constexpr MIN_WEIGHT = 0;


/*
 * generate random integer in closed range [lo, hi]
 */
template<typename T, typename Gen>
T random_int(Gen & gen, T lo, T hi)
{
    return std::uniform_int_distribution<T>(lo, hi)(gen);
};


auto gen_number_of_features(int max_num_of_features = MAX_NUM_OF_FEATURES) -> Tsetlini::number_of_features_t
{
    return Tsetlini::number_of_features_t{*rc::gen::inRange(1, max_num_of_features + 1)};
}

auto gen_number_of_clause_outputs(int max_num_of_clause_outputs = MAX_NUM_OF_CLAUSE_OUTPUTS) -> Tsetlini::number_of_estimator_clause_outputs_t
{
    return Tsetlini::number_of_estimator_clause_outputs_t{2 * *rc::gen::inRange(1, max_num_of_clause_outputs / 2 + 1)};
}

auto gen_number_of_states() -> Tsetlini::number_of_states_t
{
    return Tsetlini::number_of_states_t{*rc::gen::inRange(1, MAX_NUM_OF_STATES + 1)};
}

auto gen_threshold() -> Tsetlini::threshold_t
{
    return Tsetlini::threshold_t{*rc::gen::inRange(1, MAX_THRESHOLD + 1)};
}

auto gen_boost_tpf()
{
    return Tsetlini::boost_tpf_t{*rc::gen::arbitrary<bool>()};
}

auto gen_box_muller_flag()
{
    return Tsetlini::box_muller_flag_t{*rc::gen::arbitrary<bool>()};
}

auto gen_positive_response_error() -> Tsetlini::response_error_t
{
    using underlying_type = strong::underlying_type_t<Tsetlini::response_error_t>;
    using gen_type = long;

    static_assert(sizeof (underlying_type) < sizeof (gen_type));

    return Tsetlini::response_error_t{*rc::gen::inRange<gen_type>(1, gen_type{MAX_RESPONSE_ERROR} + 1)};
}

auto gen_negative_response_error() -> Tsetlini::response_error_t
{
    using underlying_type = strong::underlying_type_t<Tsetlini::response_error_t>;
    using gen_type = long;

    static_assert(sizeof (underlying_type) < sizeof (gen_type));

    return Tsetlini::response_error_t{*rc::gen::inRange<gen_type>(gen_type{MIN_RESPONSE_ERROR}, 0)};
}

auto gen_S_inv() -> Tsetlini::real_type
{
    return *rc::gen::map(rc::gen::arbitrary<std::uint32_t>(), [](auto x){ return (x + 0.5f) * (1.0f / 4294967296.0f); });
}

auto gen_arbitrary_clause_output(Tsetlini::number_of_estimator_clause_outputs_t number_of_clause_outputs)
{
    return *rc::gen::container<Tsetlini::aligned_vector_char>(value_of(number_of_clause_outputs), rc::gen::arbitrary<bool>());
}

auto gen_arbitrary_X(Tsetlini::number_of_features_t number_of_features)
{
    Tsetlini::bit_vector_uint64 X(value_of(number_of_features));

    X.m_vector = *rc::gen::container<Tsetlini::bit_vector_uint64::aligned_vector>(X.m_vector.size(), rc::gen::arbitrary<Tsetlini::bit_vector_uint64::block_type>());

    // clear any random bits beyond valid index range
    auto const total_bits = X.m_vector.size() * X.block_bits;
    for (Tsetlini::size_type ix = value_of(number_of_features); ix < total_bits; ++ix)
    {
        X.clear(ix);
    }

    return X;
}

template<typename PRNG>
auto gen_arbitrary_X(PRNG & irng, Tsetlini::number_of_features_t number_of_features)
{
    Tsetlini::bit_vector_uint64 X(value_of(number_of_features));

    std::generate(X.m_vector.begin(), X.m_vector.end(), irng);

    auto const total_bits = X.m_vector.size() * X.block_bits;
    for (Tsetlini::size_type ix = value_of(number_of_features); ix < total_bits; ++ix)
    {
        X.clear(ix);
    }

    return X;
}

auto gen_random_loss_fn = []
{
    return [](float)
        {
            return *rc::gen::arbitrary<float>();
        };
};

auto make_fixed_loss_fn = [](float rv)
{
    return [rv](float)
        {
            return rv;
        };
};


using coin_tosser_type = Tsetlini::RegressorStateBitwise::cache_type::coin_tosser_type;
using matrix_type = Tsetlini::numeric_matrix_int16;
using polarity_matrix_type = Tsetlini::bit_matrix_uint64;

auto gen_ta_state_matrix(
    Tsetlini::number_of_estimator_clause_outputs_t number_of_clause_outputs,
    Tsetlini::number_of_features_t number_of_features,
    int const lo_closed,
    int const hi_open) -> matrix_type
{
    matrix_type ta_state_matrix(2 * value_of(number_of_clause_outputs), value_of(number_of_features));

    // fill entire matrix storage space, regardless of alignment and padding
    ta_state_matrix.m_v =
        *rc::gen::container<matrix_type::aligned_vector>(ta_state_matrix.m_v.size(), rc::gen::inRange<matrix_type::value_type>(lo_closed, hi_open));

    return ta_state_matrix;
}

auto make_polarity_matrix_from(matrix_type const & ta_state_matrix)
{
    auto const [nrows, ncols] = ta_state_matrix.shape();

    polarity_matrix_type polarity(nrows, ncols);

    for (auto rix = 0u; rix < nrows; ++rix)
    {
        for (auto cix = 0u; cix < ncols; ++cix)
        {
            // x >= 0  --> 1
            // x < 0   --> 0
            auto const negative = ta_state_matrix[{rix, cix}] < 0;

            if (negative)
            {
                polarity.clear(rix, cix);
            }
            else
            {
                polarity.set(rix, cix);
            }
        }
    }

    return polarity;
};

auto verify_polarities(polarity_matrix_type const & polarity, matrix_type const & ta_state_matrix) -> bool
{
    /* check that every polarity reflects values iof TA state matrix */
    auto const [nrows, ncols] = ta_state_matrix.shape();
    bool all_ok = true;

    for (auto rix = 0u; rix < nrows; ++rix)
    {
        for (auto cix = 0u; cix < ncols; ++cix)
        {
            auto const negative = ta_state_matrix[{rix, cix}] < 0;

            all_ok = all_ok and
                (
                    (negative == true and polarity.test(rix, cix) == 0) or
                    (negative == false and polarity.test(rix, cix) == 1)
                );
        }
    }

    return all_ok;
}

template<typename T>
void aggregate_diff(
    Tsetlini::w_vector_type const & weight_vector,
    Tsetlini::w_vector_type const & reference_vector,
    Tsetlini::aligned_vector<T> & diff)
{
    for (auto ix = 0u; ix < weight_vector.size(); ++ix)
    {
        diff[ix] += (weight_vector[ix] - reference_vector[ix]);
    }
}

template<typename T>
void aggregate_diff(
    matrix_type const & ta_state_matrix,
    matrix_type const & reference_matrix,
    Tsetlini::numeric_matrix<T> & diff)
{
    for (auto rix = 0u; rix < ta_state_matrix.rows(); ++rix)
    {
        for (auto cix = 0u; cix < ta_state_matrix.cols(); ++cix)
        {
            diff.row_data(rix)[cix] += (ta_state_matrix[{rix, cix}] - reference_matrix[{rix, cix}]);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////


suite TrainRegressorAutomata = []
{


/*
 * Response: 0
 * Clause outputs: n/a
 * X: n/a
 */

"Bitwise non-weighted train_regressor_automata"
" does not modify TA state"
" when response error is 0"_test = [&]
{
    auto ok = rc::check(
        [&]
        {
            IRNG prng(*rc::gen::arbitrary<int>());

            auto const number_of_features = gen_number_of_features();
            auto const number_of_clause_outputs = gen_number_of_clause_outputs();

            auto const number_of_states = gen_number_of_states();
            auto const boost_tpf = gen_boost_tpf();
            auto const threshold = gen_threshold();
            auto const box_muller_flag = gen_box_muller_flag();
            auto const S_inv = gen_S_inv();
            auto const max_weight = Tsetlini::max_weight_t{0};

            auto const ta_state_reference = gen_ta_state_matrix(number_of_clause_outputs, number_of_features, -value_of(number_of_states), value_of(number_of_states));
            auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);
            auto const clause_output = gen_arbitrary_clause_output(number_of_clause_outputs);
            auto const X = gen_arbitrary_X(number_of_features);
            auto const loss_fn = gen_random_loss_fn();
            Tsetlini::w_vector_type empty_weights;

            coin_tosser_type ct(S_inv, value_of(number_of_features));

            Tsetlini::response_error_t zero_response_error{0};

            matrix_type ta_state = ta_state_reference;
            polarity_matrix_type polarity = polarity_reference;

            Tsetlini::train_regressor_automata(
                ta_state,
                polarity,
                empty_weights,
                0, value_of(number_of_clause_outputs),
                clause_output.data(),
                number_of_states,
                zero_response_error,
                X,
                max_weight,
                loss_fn,
                box_muller_flag,
                boost_tpf, prng, threshold, ct);

            RC_ASSERT(ta_state.m_v == ta_state_reference.m_v);
            RC_ASSERT(polarity.m_v == polarity_reference.m_v);
        }
    );

    expect(that % true == ok);
};


"Bitwise weighted train_regressor_automata"
" does not modify TA state"
" when response error is 0"_test = [&]
{
    auto ok = rc::check(
        [&]
        {
            IRNG prng(*rc::gen::arbitrary<int>());

            auto const number_of_features = gen_number_of_features();
            auto const number_of_clause_outputs = gen_number_of_clause_outputs();

            auto const number_of_states = gen_number_of_states();
            auto const boost_tpf = gen_boost_tpf();
            auto const threshold = gen_threshold();
            auto const box_muller_flag = gen_box_muller_flag();
            auto const S_inv = gen_S_inv();
            auto const max_weight = Tsetlini::max_weight_t{0};

            auto const ta_state_reference = gen_ta_state_matrix(number_of_clause_outputs, number_of_features, -value_of(number_of_states), value_of(number_of_states));
            auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);
            auto const clause_output = gen_arbitrary_clause_output(number_of_clause_outputs);
            auto const X = gen_arbitrary_X(number_of_features);
            auto const loss_fn = gen_random_loss_fn();
            auto const weights_reference = *rc::gen::container<Tsetlini::w_vector_type>(value_of(number_of_clause_outputs),
                rc::gen::inRange(MIN_WEIGHT, MAX_WEIGHT));

            coin_tosser_type ct(S_inv, value_of(number_of_features));

            Tsetlini::response_error_t zero_response_error{0};

            matrix_type ta_state = ta_state_reference;
            polarity_matrix_type polarity = polarity_reference;
            Tsetlini::w_vector_type weights = weights_reference;

            Tsetlini::train_regressor_automata(
                ta_state,
                polarity,
                weights,
                0, value_of(number_of_clause_outputs),
                clause_output.data(),
                number_of_states,
                zero_response_error,
                X,
                max_weight,
                loss_fn,
                box_muller_flag,
                boost_tpf, prng, threshold, ct);

            RC_ASSERT(ta_state.m_v == ta_state_reference.m_v);
            RC_ASSERT(polarity.m_v == polarity_reference.m_v);
            RC_ASSERT(weights == weights_reference);
        }
    );

    expect(that % true == ok);
};


/*
 * Response error: Positive
 * Clause outputs: 0
 * X: n/a
 */

"Bitwise non-weighted train_regressor_automata"
" does not modify TA state"
" when response error is positive"
" and clause outputs are 0"_test = [&]
{
    auto ok = rc::check(
        [&]
        {
            IRNG prng(*rc::gen::arbitrary<int>());

            auto const number_of_features = gen_number_of_features();
            auto const number_of_clause_outputs = gen_number_of_clause_outputs();

            auto const number_of_states = gen_number_of_states();
            auto const boost_tpf = gen_boost_tpf();
            auto const threshold = gen_threshold();
            auto const box_muller_flag = gen_box_muller_flag();
            auto const S_inv = gen_S_inv();
            auto const max_weight = Tsetlini::max_weight_t{0};

            auto const ta_state_reference = gen_ta_state_matrix(number_of_clause_outputs, number_of_features, -value_of(number_of_states), value_of(number_of_states));
            auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);
            auto const X = gen_arbitrary_X(number_of_features);
            auto const loss_fn = gen_random_loss_fn();
            Tsetlini::w_vector_type empty_weights;

            coin_tosser_type ct(S_inv, value_of(number_of_features));

            Tsetlini::aligned_vector_char const clause_output(value_of(number_of_clause_outputs), 0);

            auto const response_error = gen_positive_response_error();

            matrix_type ta_state = ta_state_reference;
            polarity_matrix_type polarity = polarity_reference;

            Tsetlini::train_regressor_automata(
                ta_state,
                polarity,
                empty_weights,
                0, value_of(number_of_clause_outputs),
                clause_output.data(),
                number_of_states,
                response_error,
                X,
                max_weight,
                loss_fn,
                box_muller_flag,
                boost_tpf, prng, threshold, ct);

            RC_ASSERT(ta_state.m_v == ta_state_reference.m_v);
            RC_ASSERT(polarity.m_v == polarity_reference.m_v);
        }
    );

    expect(that % true == ok);
};


"Bitwise weighted train_regressor_automata"
" does not modify TA state nor weights"
" when response error is positive"
" and clause outputs are 0"_test = [&]
{
    auto ok = rc::check(
        [&]
        {
            IRNG prng(*rc::gen::arbitrary<int>());

            auto const number_of_features = gen_number_of_features();
            auto const number_of_clause_outputs = gen_number_of_clause_outputs();

            auto const number_of_states = gen_number_of_states();
            auto const boost_tpf = gen_boost_tpf();
            auto const threshold = gen_threshold();
            auto const box_muller_flag = gen_box_muller_flag();
            auto const S_inv = gen_S_inv();

            auto const ta_state_reference = gen_ta_state_matrix(number_of_clause_outputs, number_of_features, -value_of(number_of_states), value_of(number_of_states));
            auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);
            auto const X = gen_arbitrary_X(number_of_features);
            auto const loss_fn = gen_random_loss_fn();
            auto const weights_reference = *rc::gen::container<Tsetlini::w_vector_type>(value_of(number_of_clause_outputs),
                rc::gen::inRange(MIN_WEIGHT, MAX_WEIGHT));

            coin_tosser_type ct(S_inv, value_of(number_of_features));

            Tsetlini::aligned_vector_char const clause_output(value_of(number_of_clause_outputs), 0);

            auto const response_error = gen_positive_response_error();

            matrix_type ta_state = ta_state_reference;
            polarity_matrix_type polarity = polarity_reference;
            Tsetlini::w_vector_type weights = weights_reference;

            Tsetlini::train_regressor_automata(
                ta_state,
                polarity,
                weights,
                0, value_of(number_of_clause_outputs),
                clause_output.data(),
                number_of_states,
                response_error,
                X,
                Tsetlini::max_weight_t{MAX_WEIGHT},
                loss_fn,
                box_muller_flag,
                boost_tpf, prng, threshold, ct);

            RC_ASSERT(ta_state.m_v == ta_state_reference.m_v);
            RC_ASSERT(polarity.m_v == polarity_reference.m_v);
            RC_ASSERT(weights == weights_reference);
        }
    );

    expect(that % true == ok);
};


auto make_ta_state_matrix = [](
    auto && gen,
    Tsetlini::number_of_estimator_clause_outputs_t number_of_clause_outputs,
    Tsetlini::number_of_features_t number_of_features)
{
    matrix_type ta_state_matrix(2 * value_of(number_of_clause_outputs), value_of(number_of_features));

    // fill entire matrix storage space, regardless of alignment and padding
    std::generate(ta_state_matrix.m_v.begin(), ta_state_matrix.m_v.end(), gen);

    return ta_state_matrix;
};


"Bitwise weighted train_regressor_automata"
" decrements weights"
" when response error is positive"
" and clause outputs are 1"_test = [&]
{
    /*
     * override few limits for faster execution
     */
    auto constexpr MAX_NUM_OF_FEATURES = 400;
    auto constexpr MAX_NUM_OF_CLAUSE_OUTPUTS = 8;

    /*
     * Begin with a PRNG section
     */
    std::random_device rd;
    auto const seed = rd();
    std::mt19937 gen(seed);

    IRNG prng(seed);

    /*
     * Initialize few random constants for the algorithm
     */
    auto const number_of_features = Tsetlini::number_of_features_t{random_int(gen, 1, MAX_NUM_OF_FEATURES)};
    auto const number_of_clause_outputs = Tsetlini::number_of_estimator_clause_outputs_t{2 * random_int(gen, 1, MAX_NUM_OF_CLAUSE_OUTPUTS / 2)};

    auto const number_of_states = Tsetlini::number_of_states_t{random_int(gen, 2, MAX_NUM_OF_STATES)};
    auto const boost_tpf = Tsetlini::boost_tpf_t{random_int(gen, 0, 1)};
    auto const S_inv = std::uniform_real_distribution<>(0.f, 1.f)(gen);
    auto const threshold = Tsetlini::threshold_t{random_int(gen, 1, MAX_THRESHOLD)};
    auto const box_muller_flag = Tsetlini::box_muller_flag_t{random_int(gen, 0, 1)};
    auto const loss_fn = make_fixed_loss_fn(1.0);

    coin_tosser_type ct(S_inv, value_of(number_of_features));

    auto const X = gen_arbitrary_X(gen, number_of_features);
    auto const ta_state_reference = make_ta_state_matrix(
        [&]{ return random_int(gen, -value_of(number_of_states) + 1, value_of(number_of_states) - 1); },
        number_of_clause_outputs, number_of_features);
    auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);

    Tsetlini::w_vector_type weights_reference(value_of(number_of_clause_outputs),
        random_int(gen, std::uint32_t(MIN_WEIGHT) + 10, std::uint32_t(MAX_WEIGHT - 1)));

    Tsetlini::aligned_vector_char const clause_output(value_of(number_of_clause_outputs), 1);
    auto const response_error = Tsetlini::response_error_t{random_int(gen, 1, MAX_RESPONSE_ERROR)};

    /*
     * Here we will aggregate differences between weights and their base reference
     */
    Tsetlini::aligned_vector_int32 diff(value_of(number_of_clause_outputs), 0);

    /*
     * Repeatedly call the algorithm and aggregate differences to the state
     */
    auto N_REPEAT = 15'000u * (value_of(number_of_clause_outputs) + 2); // empirical

    for (auto it = 0u; it < N_REPEAT; ++it)
    {
        matrix_type ta_state = ta_state_reference;
        polarity_matrix_type polarity = polarity_reference;
        Tsetlini::w_vector_type weights = weights_reference;

        Tsetlini::train_regressor_automata(
            ta_state,
            polarity,
            weights,
            0, value_of(number_of_clause_outputs),
            clause_output.data(),
            number_of_states,
            response_error,
            X,
            Tsetlini::max_weight_t{MAX_WEIGHT},
            loss_fn,
            box_muller_flag,
            boost_tpf, prng, threshold, ct);

        aggregate_diff(weights, weights_reference, diff);
    }

    int const target = -N_REPEAT;

    /*
     * Check that no weight diff element deviates from target by more than
     * a margin of N_REPEAT / 100
     */
    auto within_margin = [margin = std::round(N_REPEAT / 100)](int target)
        {
            return [=](auto x)
                {
                    return (target - margin) <= x and x <= (target + margin);
                };
        };

    auto const where_failed = std::find_if_not(diff.cbegin(), diff.cend(), within_margin(target));

    if (where_failed != diff.cend())
    {
        boost::ut::log << "Random seed: " << seed;
        boost::ut::log << "Number of clause_outputs: " << diff.size();
        boost::ut::log << "Target adjustment: " << target;

        boost::ut::log << "Failed element: " << *where_failed << " @ [" << (where_failed - diff.cbegin()) << ']';
    }

    expect(that % true == (where_failed == diff.cend())) << "Decrementation of weights failed!";
};


"Bitwise weighted train_regressor_automata"
" does not decrement zero weights"
" when response error is positive"
" and clause outputs are 1"_test = [&]
{
    /*
     * override few limits for faster execution
     */
    auto constexpr MAX_NUM_OF_FEATURES = 400;
    auto constexpr MAX_NUM_OF_CLAUSE_OUTPUTS = 8;

    /*
     * Begin with a PRNG section
     */
    std::random_device rd;
    auto const seed = rd();
    std::mt19937 gen(seed);

    IRNG prng(seed);

    /*
     * Initialize few random constants for the algorithm
     */
    auto const number_of_features = Tsetlini::number_of_features_t{random_int(gen, 1, MAX_NUM_OF_FEATURES)};
    auto const number_of_clause_outputs = Tsetlini::number_of_estimator_clause_outputs_t{2 * random_int(gen, 1, MAX_NUM_OF_CLAUSE_OUTPUTS / 2)};

    auto const number_of_states = Tsetlini::number_of_states_t{random_int(gen, 2, MAX_NUM_OF_STATES)};
    auto const boost_tpf = Tsetlini::boost_tpf_t{random_int(gen, 0, 1)};
    auto const S_inv = std::uniform_real_distribution<>(0.f, 1.f)(gen);
    auto const threshold = Tsetlini::threshold_t{random_int(gen, 1, MAX_THRESHOLD)};
    auto const box_muller_flag = Tsetlini::box_muller_flag_t{random_int(gen, 0, 1)};
    auto const loss_fn = make_fixed_loss_fn(1.0);

    coin_tosser_type ct(S_inv, value_of(number_of_features));

    auto const X = gen_arbitrary_X(gen, number_of_features);
    auto const ta_state_reference = make_ta_state_matrix(
        [&]{ return random_int(gen, -value_of(number_of_states) + 1, value_of(number_of_states) - 1); },
        number_of_clause_outputs, number_of_features);
    auto const polarity_reference = make_polarity_matrix_from(ta_state_reference);

    Tsetlini::w_vector_type weights_reference(value_of(number_of_clause_outputs), MIN_WEIGHT);

    Tsetlini::aligned_vector_char const clause_output(value_of(number_of_clause_outputs), 1);
    auto const response_error = Tsetlini::response_error_t{random_int(gen, 1, MAX_RESPONSE_ERROR)};

    /*
     * Repeatedly call the algorithm and check invariants
     */
    auto N_REPEAT = 10'000u;

    bool all_ok = true;

    for (auto it = 0u; it < N_REPEAT; ++it)
    {
        matrix_type ta_state = ta_state_reference;
        polarity_matrix_type polarity = polarity_reference;
        Tsetlini::w_vector_type weights = weights_reference;

        Tsetlini::train_regressor_automata(
            ta_state,
            polarity,
            weights,
            0, value_of(number_of_clause_outputs),
            clause_output.data(),
            number_of_states,
            response_error,
            X,
            Tsetlini::max_weight_t{MAX_WEIGHT},
            loss_fn,
            box_muller_flag,
            boost_tpf, prng, threshold, ct);

        all_ok = all_ok and (weights == weights_reference);
    }

    expect(that % true == all_ok) << "Weights were modified!";
};


}; // suite TrainRegressorAutomata


int main()
{
    auto failed = cfg<>.run({.report_errors = true});

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

#pragma once

#include "tsetlin_state.hpp"
#include "tsetlin_types.hpp"

namespace Tsetlin
{

namespace
{


inline
int pos_feat_index(int k)
{
    return k;
}


inline
int neg_feat_index(int k, int number_of_features)
{
    return k + number_of_features;
}


template<typename state_type>
inline
bool action(state_type state)
{
    return state >= 0;
}


inline
int pos_clause_index(int target_label, int j, int number_of_pos_neg_clauses_per_label)
{
    return 2 * target_label * number_of_pos_neg_clauses_per_label + j;
}


inline
int neg_clause_index(int target_label, int j, int number_of_pos_neg_clauses_per_label)
{
    return pos_clause_index(target_label, j, number_of_pos_neg_clauses_per_label) + number_of_pos_neg_clauses_per_label;
}


inline
void sum_up_label_votes(
    aligned_vector_char const & clause_output,
    aligned_vector_int & label_sum,
    int target_label,

    int const number_of_pos_neg_clauses_per_label,
    int const threshold)
{
    label_sum[target_label] = 0;

    for (int j = 0; j < number_of_pos_neg_clauses_per_label; ++j)
    {
        label_sum[target_label] += clause_output[pos_clause_index(target_label, j, number_of_pos_neg_clauses_per_label)];
    }

    for (int j = 0; j < number_of_pos_neg_clauses_per_label; ++j)
    {
        label_sum[target_label] -= clause_output[neg_clause_index(target_label, j, number_of_pos_neg_clauses_per_label)];
    }
    label_sum[target_label] = std::clamp(label_sum[target_label], -threshold, threshold);
}


inline
void sum_up_all_class_votes(
    aligned_vector_char const & clause_output,
    aligned_vector_int & label_sum,

    int const number_of_labels,
    int const number_of_pos_neg_clauses_per_label,
    int const threshold)
{
    for (int target_label = 0; target_label < number_of_labels; ++target_label)
    {
        sum_up_label_votes(clause_output, label_sum, target_label, number_of_pos_neg_clauses_per_label, threshold);
    }
}


template<typename state_type>
inline
void calculate_clause_output_for_predict(
    aligned_vector_char const & X,
    aligned_vector_char & clause_output,
    int const number_of_clauses,
    int const number_of_features,
    std::vector<aligned_vector<state_type>> const & ta_state)
{
    char const * X_p = assume_aligned<alignment>(X.data());

    for (int j = 0; j < number_of_clauses; ++j)
    {
        bool output = true;
        bool all_exclude = true;

        state_type const * ta_state_j = assume_aligned<alignment>(ta_state[j].data());

        for (int k = 0; k < number_of_features and output == true; ++k)
        {
            bool const action_include = action(ta_state_j[pos_feat_index(k)]);
            bool const action_include_negated = action(ta_state_j[neg_feat_index(k, number_of_features)]);

            all_exclude = (action_include == true or action_include_negated == true) ? false : all_exclude;

            output = ((action_include == true and X_p[k] == 0) or (action_include_negated == true and X_p[k] != 0)) ? false : output;
        }

        output = (all_exclude == true) ? false : output;

        clause_output[j] = output;
    }
}


template<typename state_type>
inline
void calculate_clause_output_OLD(
    aligned_vector_char const & X,
    aligned_vector_char & clause_output,
    int const number_of_clauses,
    int const number_of_features,
    std::vector<aligned_vector<state_type>> const & ta_state)
{
    char const * X_p = assume_aligned<alignment>(X.data());

    for (int j = 0; j < number_of_clauses; ++j)
    {
        bool output = true;

        state_type const * ta_state_j = assume_aligned<alignment>(ta_state[j].data());

        for (int k = 0; k < number_of_features and output == true; ++k)
        {
            bool const action_include = action(ta_state_j[pos_feat_index(k)]);
            bool const action_include_negated = action(ta_state_j[neg_feat_index(k, number_of_features)]);

            output = ((action_include == true and X_p[k] == 0) or (action_include_negated == true and X_p[k] != 0)) ? false : output;
        }

        clause_output[j] = output;
    }
}


template<typename state_type, int BATCH_SZ=16>
inline
void calculate_clause_output(
    aligned_vector_char const & X,
    aligned_vector_char & clause_output,
    int const number_of_clauses,
    int const number_of_features,
    std::vector<aligned_vector<state_type>> const & ta_state)
{
    char const * X_p = assume_aligned<alignment>(X.data());

    if (number_of_features < BATCH_SZ)
    {
        for (int j = 0; j < number_of_clauses; ++j)
        {
            bool output = true;

            state_type const * ta_state_j = assume_aligned<alignment>(ta_state[j].data());

            for (int k = 0; k < number_of_features and output == true; ++k)
            {
                bool const action_include = action(ta_state_j[pos_feat_index(k)]);
                bool const action_include_negated = action(ta_state_j[neg_feat_index(k, number_of_features)]);

                output = ((action_include == true and X_p[k] == 0) or (action_include_negated == true and X_p[k] != 0)) ? false : output;
            }

            clause_output[j] = output;
        }
    }
    else
    {
        for (int j = 0; j < number_of_clauses; ++j)
        {
            char toggle_output = 0;

            state_type const * ta_state_j = assume_aligned<alignment>(ta_state[j].data());

            int kk = 0;
            for (; kk < number_of_features - (BATCH_SZ - 1); kk += BATCH_SZ)
            {
                for (int k = kk; k < BATCH_SZ + kk; ++k)
                {
                    bool const action_include = action(ta_state_j[pos_feat_index(k)]);
                    bool const action_include_negated = action(ta_state_j[neg_feat_index(k, number_of_features)]);

                    char flag = ((X_p[k] | !action_include) ^ 1) | (((!action_include_negated) | (X_p[k] ^ 1)) ^ 1);
                    toggle_output = flag > toggle_output ? flag : toggle_output;
                }
                if (toggle_output != 0)
                {
                    break;
                }
            }
            for (int k = kk; k < number_of_features and toggle_output == false; ++k)
            {
                bool const action_include = action(ta_state_j[pos_feat_index(k)]);
                bool const action_include_negated = action(ta_state_j[neg_feat_index(k, number_of_features)]);

                char flag = ((X_p[k] | !action_include) ^ 1) | (((!action_include_negated) | (X_p[k] ^ 1)) ^ 1);
                toggle_output = flag > toggle_output ? flag : toggle_output;
            }

            clause_output[j] = !toggle_output;
        }
    }
}


// Feedback Type I, negative
template<typename state_type>
int block1(
    int const number_of_features,
    int const number_of_states,
    float const S_inv,
    state_type * __restrict ta_state_j,
    float const * __restrict fcache,
    int fcache_pos
)
{
    fcache = assume_aligned<alignment>(fcache);
    ta_state_j = assume_aligned<alignment>(ta_state_j);

    for (int k = 0; k < number_of_features; ++k)
    {
        {
            auto cond = fcache[fcache_pos++] <= S_inv;
            auto tix = pos_feat_index(k);

            ta_state_j[tix] = cond ? (ta_state_j[tix] > -number_of_states ? ta_state_j[tix] - 1 : ta_state_j[tix]) : ta_state_j[tix];
        }

        {
            auto cond = fcache[fcache_pos++] <= S_inv;
            auto tix = neg_feat_index(k, number_of_features);
            ta_state_j[tix] = cond ? (ta_state_j[tix] > -number_of_states ? ta_state_j[tix] - 1 : ta_state_j[tix]) : ta_state_j[tix];
        }
    }
    return fcache_pos;
}


// Feedback Type I, positive
template<bool boost_true_positive_feedback, typename state_type>
int block2(
    int const number_of_features,
    int const number_of_states,
    float const S_inv,
    state_type * __restrict ta_state_j,
    char const * __restrict X,
    float const * __restrict fcache,
    int fcache_pos
)
{
    constexpr float ONE = 1.0f;
    fcache = assume_aligned<alignment>(fcache);
    ta_state_j = assume_aligned<alignment>(ta_state_j);
//    X = assume_aligned(X);

    for (int k = 0; k < number_of_features; ++k)
    {
        auto cond1 = boost_true_positive_feedback == true or (fcache[fcache_pos++] <= (ONE - S_inv));
        auto cond2 = fcache[fcache_pos++] <= S_inv;

        if (X[k] != 0)
        {
            if (cond1)
            {
                if (ta_state_j[pos_feat_index(k)] < number_of_states - 1)
                {
                    ta_state_j[pos_feat_index(k)]++;
                }
            }
            if (cond2)
            {
                if (ta_state_j[neg_feat_index(k, number_of_features)] > -number_of_states)
                {
                    ta_state_j[neg_feat_index(k, number_of_features)]--;
                }
            }
        }
        else // if (X[k] == 0)
        {
            if (cond1)
            {
                if (ta_state_j[neg_feat_index(k, number_of_features)] < number_of_states - 1)
                {
                    ta_state_j[neg_feat_index(k, number_of_features)]++;
                }
            }

            if (cond2)
            {
                if (ta_state_j[pos_feat_index(k)] > -number_of_states)
                {
                    ta_state_j[pos_feat_index(k)]--;
                }
            }
        }
    }

    return fcache_pos;
}


// Feedback Type II
template<typename state_type>
void block3(
    int const number_of_features,
    state_type * __restrict ta_state_j,
    char const * __restrict X
)
{
    ta_state_j = assume_aligned<alignment>(ta_state_j);
    X = assume_aligned<alignment>(X);

    for (int k = 0; k < number_of_features; ++k)
    {
        if (X[k] == 0)
        {
            auto tix = pos_feat_index(k);
            auto action_include = (ta_state_j[tix]) >= 0;
            if (action_include == false)
            {
                ta_state_j[tix]++;
            }
        }
        else //if(X[k] == 1)
        {
            auto tix = neg_feat_index(k, number_of_features);
            auto action_include_negated = (ta_state_j[tix]) >= 0;
            if (action_include_negated == false)
            {
                ta_state_j[tix]++;
            }
        }
    }
#if 0
    for (int k = 0; k < number_of_features; ++k)
    {
        bool const action_include = action(ta_state[j][pos_feat_index(k)]);
        bool const action_include_negated = action(ta_state[j][neg_feat_index(k)]);

        if (X[k] == 0)
        {
            if (action_include == false and ta_state[j][pos_feat_index(k)] < number_of_states * 2)
            {
                ta_state[j][pos_feat_index(k)]++;
            }
        }
        else if(X[k] == 1)
        {
            if (action_include_negated == false and ta_state[j][neg_feat_index(k)] < number_of_states * 2)
            {
                ta_state[j][neg_feat_index(k)]++;
            }
        }
    }
#endif
}


template<typename state_type>
void train_automata_batch(
    aligned_vector<state_type> * __restrict ta_state,
    int const begin,
    int const end,
    feedback_vector_type::value_type const * __restrict feedback_to_clauses,
    char const * __restrict clause_output,
    int const number_of_features,
    int const number_of_states,
    float const S_inv,
    char const * __restrict X,
    bool const boost_true_positive_feedback,
    ClassifierState::frand_cache_type & fcache
    )
{
    float const * fcache_ = assume_aligned<alignment>(fcache.m_fcache.data());

    for (int j = begin; j < end; ++j)
    {
        state_type * ta_state_j = ::assume_aligned<alignment>(ta_state[j].data());

        if (feedback_to_clauses[j] > 0)
        {
            if (clause_output[j] == 0)
            {
                fcache.refill();

                fcache.m_pos = block1(number_of_features, number_of_states, S_inv, ta_state_j, fcache_, fcache.m_pos);
            }
            else if (clause_output[j] == 1)
            {
                fcache.refill();

                if (boost_true_positive_feedback)
                    fcache.m_pos = block2<true>(number_of_features, number_of_states, S_inv, ta_state_j, X, fcache_, fcache.m_pos);
                else
                    fcache.m_pos = block2<false>(number_of_features, number_of_states, S_inv, ta_state_j, X, fcache_, fcache.m_pos);
            }
        }
        else if (feedback_to_clauses[j] < 0)
        {
            if (clause_output[j] == 1)
            {
                block3(number_of_features, ta_state_j, X);
            }
        }
    }
}

}

}

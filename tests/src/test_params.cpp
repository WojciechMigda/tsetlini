#include "tsetlin_params.hpp"
#include "tsetlin_types.hpp"

#include <gtest/gtest.h>
#include <variant>
#include <thread>
#include <algorithm>

namespace
{


TEST(Params, can_be_created)
{
    auto const rv = Tsetlin::make_params_from_json();

    EXPECT_TRUE(rv);
}


TEST(Params, cannot_be_created_from_empty_string_json)
{
    auto const rv = Tsetlin::make_params_from_json("");

    EXPECT_FALSE(rv);
}


TEST(Params, can_be_created_from_empty_dict_json)
{
    auto const rv = Tsetlin::make_params_from_json("{}");

    EXPECT_TRUE(rv);
}


TEST(Params, cannot_be_created_from_invalid_json)
{
    auto const rv = Tsetlin::make_params_from_json("[]");

    EXPECT_FALSE(rv);
}


TEST(Params, cannot_be_created_from_malformed_json)
{
    auto const rv = Tsetlin::make_params_from_json("5\"}");

    EXPECT_FALSE(rv);
}


TEST(Params, can_be_created_from_json_with_one_integer_item)
{
    auto const rv = Tsetlin::make_params_from_json(R"({"number_of_states": 200})");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    EXPECT_EQ(200, std::get<int>(params.at("number_of_states")));
}


TEST(Params, can_be_created_from_json_with_one_float_item)
{
    auto const rv = Tsetlin::make_params_from_json(R"({"s": 3.9})");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    EXPECT_FLOAT_EQ(3.9, std::get<Tsetlin::real_type>(params.at("s")));
}


TEST(Params, can_be_created_from_json_with_one_boolean_item)
{
    auto const rv = Tsetlin::make_params_from_json(R"({"verbose": true})");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    EXPECT_EQ(true, std::get<bool>(params.at("verbose")));
}


TEST(Params, cannot_be_created_from_json_with_unrecognized_item)
{
    auto const rv = Tsetlin::make_params_from_json(R"({"foobar": true})");

    EXPECT_FALSE(rv);
}


TEST(Params, can_be_created_from_json_with_full_config)
{
    auto const rv = Tsetlin::make_params_from_json(R"(
{
"verbose": true,
"number_of_pos_neg_clauses_per_label": 17,
"number_of_states": 125,
"s": 6.3 ,
"threshold": 8,
"boost_true_positive_feedback": 1,
"n_jobs": 3,
"random_state": 123
}
)");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    EXPECT_EQ(true, std::get<bool>(params.at("verbose")));
    EXPECT_EQ(17, std::get<int>(params.at("number_of_pos_neg_clauses_per_label")));
    EXPECT_EQ(125, std::get<int>(params.at("number_of_states")));
    EXPECT_EQ(8, std::get<int>(params.at("threshold")));
    EXPECT_EQ(3, std::get<int>(params.at("n_jobs")));
    EXPECT_EQ(1, std::get<int>(params.at("boost_true_positive_feedback")));
    EXPECT_FLOAT_EQ(6.3, std::get<Tsetlin::real_type>(params.at("s")));
    EXPECT_EQ(123u, std::get<Tsetlin::seed_type>(params.at("random_state")));
}


TEST(Params, n_jobs_equal_neg_one_is_normalized)
{
    auto const rv = Tsetlin::make_params_from_json(R"({"n_jobs": -1})");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    EXPECT_FLOAT_EQ(
        std::max<int>(1, std::thread::hardware_concurrency()),
        std::get<int>(params.at("n_jobs")));
}


TEST(Params, unspecified_random_state_is_initialized)
{
    auto const rv = Tsetlin::make_params_from_json(R"({})");

    EXPECT_TRUE(rv);

    auto params = rv.right().value;

    auto random_state = params.at("random_state");

    EXPECT_TRUE(std::holds_alternative<Tsetlin::seed_type>(random_state));
}


}
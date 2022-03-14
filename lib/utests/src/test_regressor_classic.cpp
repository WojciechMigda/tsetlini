#include "tsetlini.hpp"
#include "tsetlini_types.hpp"
#include "tsetlini_strong_params.hpp"
#include "either.hpp"
#include "params_companion.hpp"

#include "boost/ut.hpp"

#include <cstdlib>
#include <optional>
#include <string>
#include <thread>
#include <limits>
#include <cmath>


using namespace boost::ut;
using namespace std::string_literals;


suite TestRegressorClassicArgs = []
{


"RegressorClassic can be created from default arguments"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic();

    expect(that % true == !!reg);
};


"RegressorClassic created from arguments can be move-assigned"_test = []
{
    std::optional<Tsetlini::RegressorClassic> maybe_estimator;

    auto const rv = Tsetlini::make_regressor_classic()
        .rightFlatMap(
        [&](auto && est)
        {
            maybe_estimator = std::move(est);

            return Tsetlini::Either<Tsetlini::status_message_t, int>::rightOf(0);
        });

    expect(that % true == maybe_estimator.has_value());
};


"RegressorClassic can be created with counting_type argument set to {int8, int16, int32, auto}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::counting_type_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::counting_type(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{"int8"s, "int16"s, "int32"s, "auto"s};


"RegressorClassic can be created with clause_output_tile_size argument set to {16, 32, 64, 128}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::clause_output_tile_size_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::clause_output_tile_size(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{16, 32, 64, 128};


"RegressorClassic cannot be created with clause_output_tile_size argument set to 24"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::clause_output_tile_size_t{24});

    expect(that % false == !!reg);
};


"RegressorClassic can be created with n_jobs argument set to 2"_test = []
{
    Tsetlini::make_regressor_classic(Tsetlini::number_of_jobs_t{2})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([](Tsetlini::RegressorClassic && clf)
            {
                expect(that % 2 == value_of(Tsetlini::Params::n_jobs(clf.read_params())));

                return std::move(clf);
            })
        ;
};


"RegressorClassic can be created with n_jobs argument set to -1"_test = []
{
    using underlying_type = strong::underlying_type_t<Tsetlini::number_of_jobs_t>;

    Tsetlini::make_regressor_classic(Tsetlini::number_of_jobs_t{-1})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([](Tsetlini::RegressorClassic && clf)
            {
                expect(that % underlying_type(std::thread::hardware_concurrency()) == value_of(Tsetlini::Params::n_jobs(clf.read_params())));

                return std::move(clf);
            })
        ;
};


"RegressorClassic cannot be created with number_of_jobs argument set to {0, -2}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::number_of_jobs_t{arg});

    expect(that % false == !!reg);
} | std::vector{0, -2};


"RegressorClassic can be created with number_of_regressor_clauses argument set to 2"_test = []
{
    Tsetlini::make_regressor_classic(Tsetlini::number_of_physical_regressor_clauses_t{2})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([](Tsetlini::RegressorClassic && clf)
            {
                expect(that % 2 == value_of(Tsetlini::Params::number_of_physical_regressor_clauses(clf.read_params())));

                return std::move(clf);
            })
        ;
};


"RegressorClassic cannot be created with number_of_regressor_clauses argument set to {1, 0, -1, -2, 13}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::number_of_physical_regressor_clauses_t{arg});

    expect(that % false == !!reg);
} | std::vector{1, 0, -1, -2, 13};


"RegressorClassic can be created with number_of_states argument set to 1"_test = []
{
    Tsetlini::make_regressor_classic(Tsetlini::number_of_states_t{1})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([](Tsetlini::RegressorClassic && clf)
            {
                expect(that % 1 == value_of(Tsetlini::Params::number_of_states(clf.read_params())));

                return std::move(clf);
            })
        ;
};


"RegressorClassic cannot be created with number_of_states argument set to {0, -1}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::number_of_states_t{arg});

    expect(that % false == !!reg);
} | std::vector{0, -1};


"RegressorClassic can be created with boost_true_positive_feedback argument set to {true, false}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::boost_tpf_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::boost_true_positive_feedback(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{true, false};


"RegressorClassic can be created with threshold argument set to {1, 2, 10}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::threshold_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::threshold(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{1, 2, 10};


"RegressorClassic cannot be created with threshold argument set to {0, -1}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::threshold_t{arg});

    expect(that % false == !!reg);
} | std::vector{0, -1};


"RegressorClassic can be created with max_weight argument set to {1, 2, 10}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::max_weight_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::max_weight(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{1, 2, 10};


"RegressorClassic cannot be created with max_weight argument set to {0, -1}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::max_weight_t{arg});

    expect(that % false == !!reg);
} | std::vector{0, -1};


"RegressorClassic can be created with verbosity argument set to {true, false}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::verbosity_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::verbose(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{true, false};


"RegressorClassic can be created with weighted argument set to {true, false}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::weighted_flag_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::weighted(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{true, false};


"RegressorClassic cannot be created with specificity argument set to {-1.0, 0.0, 1 - epsilon -inf, +inf, -NaN, +NaN}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::specificity_t{arg});

    expect(that % false == !!reg);
} | std::vector<strong::underlying_type_t<Tsetlini::specificity_t>>{
    -1.0, 0.0,
    strong::underlying_type_t<Tsetlini::specificity_t>{1} - std::numeric_limits<strong::underlying_type_t<Tsetlini::specificity_t>>::epsilon(),
    -std::numeric_limits<strong::underlying_type_t<Tsetlini::specificity_t>>::infinity(),
    std::numeric_limits<strong::underlying_type_t<Tsetlini::specificity_t>>::infinity(),
    NAN, -NAN
};


"RegressorClassic can be created with specificity argument set to {1.0, 3.14}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::specificity_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::s(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector<strong::underlying_type_t<Tsetlini::specificity_t>>{1.0, 3.14};


"RegressorClassic can be created with random_seed argument set to {1, 1234}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::random_seed_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::random_state(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{1u, 1234u};


"RegressorClassic can be created without random_seed argument"_test = []
{
    Tsetlini::make_regressor_classic()
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([](Tsetlini::RegressorClassic && clf)
            {
                expect(that % true == (clf.read_params().find("random_state") != clf.read_params().cend()));

                return std::move(clf);
            })
        ;
};


"RegressorClassic can be created with loss_fn argument set to {L2, MSE, MAE, L1, berHu, L1+2}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::loss_fn_name_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::loss_fn_name(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{"L2"s, "MSE"s, "MAE"s, "L1"s, "berHu"s, "L1+2"s};


"RegressorClassic cannot be created with loss_fn argument set to {strlen, add}"_test = [](auto const & arg)
{
    auto const reg = Tsetlini::make_regressor_classic(Tsetlini::loss_fn_name_t{arg});

    expect(that % false == !!reg);
} | std::vector{"strlen"s, "add"s};


"RegressorClassic can be created with loss_fn_C1 argument set to {-1, 0, 1234}"_test = [](auto const & arg)
{
    Tsetlini::make_regressor_classic(Tsetlini::loss_fn_C1_t{arg})
        .leftMap([](Tsetlini::status_message_t && msg)
            {
                expect(false);

                return std::move(msg);
            })
        .rightMap([&arg](Tsetlini::RegressorClassic && clf)
            {
                expect(that % arg == value_of(Tsetlini::Params::loss_fn_C1(clf.read_params())));

                return std::move(clf);
            })
        ;
} | std::vector{-1.0, 0., 1234.};


"RegressorClassic can be created with arbitrarily ordered arguments"_test = []
{
    auto const clf = Tsetlini::make_regressor_classic(
        Tsetlini::number_of_jobs_t{3},
        Tsetlini::verbosity_t{true},
        Tsetlini::number_of_physical_regressor_clauses_t{36},
        Tsetlini::number_of_states_t{125},
        Tsetlini::specificity_t{6.3},
        Tsetlini::threshold_t{8},
        Tsetlini::weighted_flag_t{true},
        Tsetlini::max_weight_t{7},
        Tsetlini::loss_fn_name_t{"MAE"},
        Tsetlini::loss_fn_C1_t{0.17},
        Tsetlini::boost_tpf_t{1},
        Tsetlini::counting_type_t{"int32"},
        Tsetlini::clause_output_tile_size_t{32},
        Tsetlini::random_seed_t{123}
    );

    expect(that % true == !!clf);
};


};


////////////////////////////////////////////////////////////////////////////////


suite TestRegressorClassicJson = []
{


"RegressorClassic can be created"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json();

    expect(that % true == !!reg);
};


"RegressorClassic can be move-assigned"_test = []
{
    std::optional<Tsetlini::RegressorClassic> maybe_estimator;

    auto const rv = Tsetlini::make_regressor_classic_from_json()
        .rightFlatMap(
        [&](auto && est)
        {
            maybe_estimator = std::move(est);

            return Tsetlini::Either<Tsetlini::status_message_t, int>::rightOf(0);
        });

    expect(that % true == maybe_estimator.has_value());
};


"RegressorClassic cannot be created from empty JSON"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json("");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from invalid JSON"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json("[]");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from empty JSON dict"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json("{}");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with unrecognized param"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"gotcha": 564})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with counting_type set to int8"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"counting_type": "int8"})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with counting_type set to int16"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"counting_type": "int16"})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with counting_type set to int32"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"counting_type": "int32"})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with counting_type set to auto"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"counting_type": "auto"})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with clause_output_tile_size set to 16"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"clause_output_tile_size": 16})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with clause_output_tile_size set to 32"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"clause_output_tile_size": 32})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with clause_output_tile_size set to 64"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"clause_output_tile_size": 64})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with clause_output_tile_size set to 128"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"clause_output_tile_size": 128})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with clause_output_tile_size set to 24"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"clause_output_tile_size": 24})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with n_jobs set to 2"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"n_jobs": 2})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with n_jobs set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"n_jobs": -1})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with n_jobs set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"n_jobs": 0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with n_jobs set to -2"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"n_jobs": -2})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with number_of_regressor_clauses set to 2"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": 2})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_regressor_clauses set to 1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": 1})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_regressor_clauses set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": 0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_regressor_clauses set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": -1})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_regressor_clauses set to -2"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": -2})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_regressor_clauses set to 13"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_regressor_clauses": 13})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with number_of_states set to 1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_states": 1})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_states set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_states": 0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with number_of_states set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"number_of_states": -1})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with boost_true_positive_feedback set to 1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"boost_true_positive_feedback": 1})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with boost_true_positive_feedback set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"boost_true_positive_feedback": 0})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with boost_true_positive_feedback set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"boost_true_positive_feedback": -1})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with boost_true_positive_feedback set to 2"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"boost_true_positive_feedback": 2})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with threshold set to 1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"threshold": 1})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with threshold set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"threshold": 0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with threshold set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"threshold": -1})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with max_weight set to 1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"max_weight": 1})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with max_weight set to 10"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"max_weight": 10})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with max_weight set to 0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"max_weight": 0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with max_weight set to -1"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"max_weight": -1})");

    expect(that % false == !!reg);
};


"RegressorClassic can be created from JSON with verbose set to true"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"verbose": true})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with verbose set to false"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"verbose": false})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with weighted set to true"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"weighted": true})");

    expect(that % true == !!reg);
};


"RegressorClassic can be created from JSON with weighted set to false"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"weighted": false})");

    expect(that % true == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to -1.0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": -1.0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to 0.0"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": 0.0})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to 0.999"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": 0.999})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to -inf"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": -inf})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to +inf"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": +inf})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to NaN"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": NaN})");

    expect(that % false == !!reg);
};


"RegressorClassic cannot be created from JSON with specificity set to nan"_test = []
{
    auto const reg = Tsetlini::make_regressor_classic_from_json(R"({"s": nan})");

    expect(that % false == !!reg);
};


}; // TestRegressorClassicJson


int main()
{
    auto failed = cfg<>.run({.report_errors = true});

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

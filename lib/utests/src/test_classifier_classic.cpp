#include "tsetlini.hpp"
#include "tsetlini_types.hpp"
#include "either.hpp"

#include "boost/ut.hpp"

#include <cstdlib>
#include <optional>


using namespace boost::ut;


suite TestClassifierClassic = []
{


"ClassifierClassic can be created"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic();

    expect(that % true == !!clf);
};


"ClassifierClassic can be move-assigned"_test = []
{
    std::optional<Tsetlini::ClassifierClassic> maybe_estimator;

    auto const rv = Tsetlini::make_classifier_classic()
        .rightFlatMap(
        [&](auto && est)
        {
            maybe_estimator = std::move(est);

            return Tsetlini::Either<Tsetlini::status_message_t, int>::rightOf(0);
        });

    expect(that % true == maybe_estimator.has_value());
};


"ClassifierClassic cannot be created from empty JSON"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic("");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from invalid JSON"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic("[]");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from empty JSON dict"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic("{}");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with unrecognized param"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"gotcha": 564})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with counting_type set to int8"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"counting_type": "int8"})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with counting_type set to int16"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"counting_type": "int16"})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with counting_type set to int32"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"counting_type": "int32"})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with counting_type set to auto"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"counting_type": "auto"})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with clause_output_tile_size set to 16"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"clause_output_tile_size": 16})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with clause_output_tile_size set to 32"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"clause_output_tile_size": 32})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with clause_output_tile_size set to 64"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"clause_output_tile_size": 64})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with clause_output_tile_size set to 128"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"clause_output_tile_size": 128})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with clause_output_tile_size set to 24"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"clause_output_tile_size": 24})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with n_jobs set to 2"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"n_jobs": 2})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with n_jobs set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"n_jobs": -1})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with n_jobs set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"n_jobs": 0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with n_jobs set to -2"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"n_jobs": -2})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with number_of_clauses_per_label set to 4"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": 4})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to 3"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": 3})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to 2"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": 2})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to 1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": 1})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": 0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": -1})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_clauses_per_label set to -4"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_clauses_per_label": -4})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with number_of_states set to 1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_states": 1})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_states set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_states": 0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with number_of_states set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"number_of_states": -1})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with boost_true_positive_feedback set to 1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"boost_true_positive_feedback": 1})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with boost_true_positive_feedback set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"boost_true_positive_feedback": 0})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with boost_true_positive_feedback set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"boost_true_positive_feedback": -1})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with boost_true_positive_feedback set to 2"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"boost_true_positive_feedback": 2})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with threshold set to 1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"threshold": 1})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with threshold set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"threshold": 0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with threshold set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"threshold": -1})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with max_weight set to 1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"max_weight": 1})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with max_weight set to 10"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"max_weight": 10})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with max_weight set to 0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"max_weight": 0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with max_weight set to -1"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"max_weight": -1})");

    expect(that % false == !!clf);
};


"ClassifierClassic can be created from JSON with verbose set to true"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"verbose": true})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with verbose set to false"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"verbose": false})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with weighted set to true"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"weighted": true})");

    expect(that % true == !!clf);
};


"ClassifierClassic can be created from JSON with weighted set to false"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"weighted": false})");

    expect(that % true == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to -1.0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": -1.0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to 0.0"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": 0.0})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to 0.999"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": 0.999})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to -inf"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": -inf})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to +inf"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": +inf})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to NaN"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": NaN})");

    expect(that % false == !!clf);
};


"ClassifierClassic cannot be created from JSON with specificity set to nan"_test = []
{
    auto const clf = Tsetlini::make_classifier_classic(R"({"s": nan})");

    expect(that % false == !!clf);
};


};

int main()
{
    auto failed = cfg<>.run({.report_errors = true});

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
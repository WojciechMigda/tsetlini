#include "tsetlini.hpp"
#include "tsetlini_types.hpp"
#include "tsetlini_strong_params.hpp"

#include "boost/ut.hpp"

#include <cstdlib>
#include <vector>


using namespace boost::ut;


suite TestRegressorClassicFit = []
{


"RegressorClassic::fit rejects empty input X"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X;
            Tsetlini::response_vector_type y{1, 0, 1, 0};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects empty input y"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y;

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects input X with rows of unequal length"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, 0, 0};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects input X with non-0/1 values"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, -1}, {0, 2, 0}};
            Tsetlini::response_vector_type y{1, 0, 0};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects input X and y with unequal dimensions"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, 0, 0, 1};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects input y with negative response"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, 0, -21};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit rejects input y with response over Threshold"_test = []
{
    auto constexpr threshold = 15;

    Tsetlini::make_regressor_classic(Tsetlini::threshold_t{threshold})
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, threshold + 1, 1};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_VALUE_ERROR == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit accepts valid input y with response equal to Threshold"_test = []
{
    auto constexpr threshold = 15;

    Tsetlini::make_regressor_classic(Tsetlini::threshold_t{threshold})
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, threshold, 1};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_OK == rv.first);

            return std::move(reg);
        });
};


"RegressorClassic::fit accepts valid input"_test = []
{
    Tsetlini::make_regressor_classic()
        .rightMap(
        [](auto && reg)
        {
            std::vector<Tsetlini::aligned_vector_char> X{{1, 0, 1}, {1, 0, 0}, {0, 0, 0}};
            Tsetlini::response_vector_type y{1, 0, 2};

            auto const rv = reg.fit(X, y);

            expect(that % Tsetlini::StatusCode::S_OK == rv.first);

            return std::move(reg);
        });
};


};

int main()
{
    auto failed = cfg<>.run({.report_errors = true});

    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

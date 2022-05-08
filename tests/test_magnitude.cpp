#include <catch2/catch.hpp>
#include <units/magnitude.hpp>


TEST_CASE("Magnitude from int", "[magnitude]")
{
    {
        using ResMag = units::MagnitudeFromInt<2 * 3 * 7 * 7 * 19>;
        using Expected = units::detail::magnitude_raw<
            units::Power<units::int_factor<2>, 1>, units::Power<units::int_factor<3>, 1>,
            units::Power<units::int_factor<7>, 2>, units::Power<units::int_factor<19>, 1>>;
        CHECK(std::is_same_v<ResMag, Expected>);
    }

    {
        using ResMag = units::MagnitudeFromInt<2L * 3 * 19 * 101 * 353 * 739>;
        using Expected = units::detail::magnitude_raw<
            units::Power<units::int_factor<2>, 1>, units::Power<units::int_factor<3>, 1>,
            units::Power<units::int_factor<19>, 1>, units::Power<units::int_factor<101>, 1>,
            units::Power<units::int_factor<353>, 1>, units::Power<units::int_factor<739>, 1>>;
        CHECK(std::is_same_v<ResMag, Expected>);
    }

    {
        using ResMag = units::MagnitudeFromInt<1>;
        using Expected = units::detail::magnitude_raw<>;
        CHECK(std::is_same_v<ResMag, Expected>);
    }
}


TEST_CASE("Inverse magnitude", "[magnitude]")
{
    {
        using Mag = units::detail::magnitude_raw<
            units::Power<units::int_factor<2>, 1>, units::Power<units::int_factor<3>, 1>,
            units::Power<units::int_factor<7>, 2>, units::Power<units::int_factor<19>, 1>,
            units::Power<units::int_factor<10007>, 1>>;
        using InvMag = units::InverseMagnitude<Mag>;
        using Expected = units::detail::magnitude_raw<
            units::Power<units::int_factor<2>, -1>, units::Power<units::int_factor<3>, -1>,
            units::Power<units::int_factor<7>, -2>, units::Power<units::int_factor<19>, -1>,
            units::Power<units::int_factor<10007>, -1>>;
        CHECK(std::is_same_v<InvMag, Expected>);
    }

    {
        using Mag = units::MagnitudeFromInt<91971>;
        using InvMag = units::InverseMagnitude<Mag>;
        using InvInvMag = units::InverseMagnitude<InvMag>;
        CHECK(std::is_same_v<Mag, InvInvMag>);
    }
}


TEST_CASE("Multiply magnitude", "[magnitude]")
{
    {
        using Mag1 = units::MagnitudeFromInt<457>;
        using Mag2 = units::MagnitudeFromInt<6874>;
        using Expected = units::MagnitudeFromInt<457 * 6874>;
        using Res = units::MultiplyMagnitude<Mag1, Mag2>;
        CHECK(std::is_same_v<Res, Expected>);
    }

    {
        using Mag = units::MagnitudeFromInt<468>;
        using InvMag = units::InverseMagnitude<Mag>;
        using Expected = units::MagnitudeFromInt<1>;
        using Res = units::MultiplyMagnitude<Mag, InvMag>;
        CHECK(std::is_same_v<Res, Expected>);
    }
}


struct MyRatio
{
    static constexpr int num = 487;
    static constexpr int den = 49;
};

TEST_CASE("Magnitude from ratio", "[magnitude]")
{
    {
        using Res = units::MagnitudeFromRatio<std::ratio<3 * 7 * 19, 5 * 11 * 41>>;
        using Expected = units::detail::magnitude_raw<
            units::Power<units::int_factor<3>, 1>, units::Power<units::int_factor<5>, -1>,
            units::Power<units::int_factor<7>, 1>, units::Power<units::int_factor<11>, -1>,
            units::Power<units::int_factor<19>, 1>, units::Power<units::int_factor<41>, -1>>;
        CHECK(std::is_same_v<Res, Expected>);
    }

    {
        using Res = units::MagnitudeFromRatio<std::ratio<420, 370>>;
        using Expected = units::MagnitudeFromRatio<std::ratio<42, 37>>;
        CHECK(std::is_same_v<Res, Expected>);
    }

    {
        using Res = units::MagnitudeFromRatio<MyRatio>;
        using Expected = units::MagnitudeFromRatio<std::ratio<487, 49>>;
        CHECK(std::is_same_v<Res, Expected>);
    }
}
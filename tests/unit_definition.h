#ifndef UNIT_DEFINITION_HPP
#define UNIT_DEFINITION_HPP

#include <ratio>
#include <units/unit.hpp>


struct Length : units::BaseDimension<Length>
{};
struct Time : units::BaseDimension<Time>
{};
struct Speed : units::CombinedDimension<Speed, units::Power<Length, 1>, units::Power<Time, -1>>
{};

struct metre : units::BaseUnit<metre, Length>
{};
struct kilometre : units::ScaledUnit<kilometre, metre, units::MagnitudeFromRatio<std::kilo>>
{};
struct millimetre : units::ScaledUnit<millimetre, metre, units::MagnitudeFromRatio<std::milli>>
{};

struct second : units::BaseUnit<second, Time>
{};
struct kilosecond : units::ScaledUnit<kilosecond, second, units::MagnitudeFromRatio<std::kilo>>
{};
struct millisecond : units::ScaledUnit<millisecond, second, units::MagnitudeFromRatio<std::milli>>
{};
struct minute : units::ScaledUnit<minute, second, units::MagnitudeFromInt<60>>
{};

struct metre_per_second : units::BaseUnit<metre_per_second, Speed>
{};

constexpr metre m;
constexpr kilometre km;
constexpr millimetre mm;
constexpr second s;
constexpr kilosecond ks;
constexpr millisecond ms;
constexpr minute min;

#endif // UNIT_DEFINITION_HPP
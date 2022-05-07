cmake_minimum_required(VERSION 3.17)
project(sycl_units)

set(CMAKE_CXX_STANDARD 17)

add_library(units INTERFACE)

target_sources(
    units
    INTERFACE include/units.hpp
              include/units/dimension.hpp
              include/units/downcast.hpp
              include/units/magnitude.hpp
              include/units/meta.hpp
              include/units/power.hpp
              include/units/primes.hpp
              include/units/quantity.hpp
              include/units/type_name.hpp
              include/units/unit.hpp
)

target_include_directories(units INTERFACE include)

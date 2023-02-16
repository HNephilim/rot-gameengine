require("export-compile-commands")

workspace("RotRenderer")
configurations({ "Debug", "Release" })

project("RotRenderer")
kind("ConsoleApp")
language("C++")
targetdir("bin/%{cfg.buildvfg}")

files({ "**.hpp", "**.cpp" })

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG" })
optimize("On")

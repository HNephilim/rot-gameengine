require("export-compile-commands")

workspace("RotRenderer")
configurations({ "Debug", "Release" })

project("RotRenderer")
kind("ConsoleApp")
language("C++")
cppdialect("C++17")
targetdir("bin/%{cfg.buildvfg}")

system("Windows")
architecture("x86_64")

files({ "main.cpp", "window/*", "app/*" })
includedirs({ "include", "C:/VulkanSDK/1.3.239.0/Include", "./" })
libdirs({ "C:/VulkanSDK/1.3.239.0/Lib", "vendor/glfw-3.3.8/lib-vc2022" })
links({ "vulkan-1", "glfw3" })

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG" })
optimize("On")

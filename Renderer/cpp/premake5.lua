require("export-compile-commands")

workspace("RotRenderer")
configurations({ "Debug", "Release" })

project("RotRenderer")
kind("ConsoleApp")
language("C++")
cppdialect("C++20")
targetdir("bin/%{cfg.buildvfg}")

system("Windows")
architecture("x86_64")

files({ "main.cpp", "profiler/*", "window/*", "app/*", "pipeline/*" })
includedirs({ "include", "C:/VulkanSDK/1.3.239.0/Include", "./" })
libdirs({ "%VULKAN_SDK%/Lib", "vendor/glfw-3.3.8/lib-vc2022" })
links({ "vulkan-1", "glfw3" })

filter("configurations:Debug")
defines({ "DEBUG", "GENERIC_PROFILER" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG", "GENERIC_PROFILER" })
optimize("On")

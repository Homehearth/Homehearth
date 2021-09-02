project "Engine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "src/**.cpp",
        "include/**.h"
    }

    includedirs {
        "include"
    }

    filter {"system:windows"}
        defines {
            "WIN32"
        }

    filter {"configurations:Debug"}
        buildoptions "/MTd"
        runtime "Debug"
        symbols "on"

    filter {"configurations:Release"}
        buildoptions "/MT"
        runtime "Release"
        optimize "on"
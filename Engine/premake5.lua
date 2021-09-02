project "Engine"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")
 
    
    -- Define the location of pre-compiled header.
    pchheader "EnginePCH.h"
	pchsource "src/EnginePCH.cpp"


    files {
        "src/**.h",
		"src/**.cpp",
    }

    includedirs {
		"src"
    }


    -- Define a macro/symbol which applies for the Windows system.
    filter {"system:windows"}
        defines {
            "WIN32"
        }


    -- Define a macro/symbol which applies only to debug builds.
    filter {"configurations:Debug"}
        --buildoptions "/MTd"
        runtime "Debug"
        defines{"_DEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        --libdirs{"ThirdParty/DirectXTK/bin/x64/Debug_lib/"}
        --links{"DirectXTK", "DirectXTKAudioWin7"}


    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        --buildoptions "/MT"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "on"
        --libdirs{"ThirdParty/DirectXTK/bin/x64/Debug_lib/"}
        --links{"DirectXTK", "DirectXTKAudioWin7"}
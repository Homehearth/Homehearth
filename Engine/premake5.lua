project "Engine"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")
 
    
    -- Define the location of pre-compiled header.
    pchheader "EnginePCH.h"
	pchsource "src/EnginePCH.cpp"


    -- Define the location(s) of files.
    -- Note: specify the path relative to the Premake file.
    files {
        "src/**.h",
		"src/**.cpp",
    }


    -- Define any includes of folder(s).
    -- Useful to remove any "../folder/file.h" in code.
    -- Note: specify the path relative to the Premake file.
    includedirs {
		"src",
        "../ThirdParty/DirectXTK/include/",
        "../ThirdParty/imGUI/",
        "../ThirdParty/stb_image/"
    }


    -- Define filter(s) inside this project. [path + NameOfFilter] { files }
    -- Create your files in 'src' folder on disk, then add them to a filter below (**filename).
    -- Note: specify the path relative to the Premake file.
    vpaths {
        ["src/Graphics"] = { },
            ["src/Graphics/Renderer"] = { },
            ["src/Graphics/D3D11"] = { },
            ["src/Graphics/DXGI"] = { },
            ["src/Graphics/Window"] = { },

        ["src/Core"] = { "**EnginePCH.h", "**EnginePCH.cpp", "**Logger.h", "**WinMain.cpp" },

        ["src/Resources"] = { },
            ["src/Resources/Shaders"] = { "**.hhlsl", "**.hlsli" },

        ["src/Audio"] = {  },
        ["src/Physics"] = {  },
        ["src/Network"] = {  },
        ["src/Animation"] = {  },
        ["src/ESC"] = {  }, 
        ["src/AI"] = {  }     
    }


    links{
        "d3d11",
        "d2d1"
    }


    -- Define a macro/symbol which applies for the Windows system.
    filter {"system:windows"}
        defines {
            "WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }


    -- Define a macro/symbol which applies only to debug builds.
    filter {"configurations:Debug"}
        --buildoptions "/MTd"
        runtime "Debug"
        defines{"_DEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        libdirs{"../ThirdParty/DirectXTK/bin/x64/Debug_lib/"}
        links{"DirectXTK", "DirectXTKAudioWin7"}


    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        --buildoptions "/MT"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "on"
        libdirs{"../DirectXTK/bin/x64/Release_lib/"}
        links{"DirectXTK", "DirectXTKAudioWin7"}
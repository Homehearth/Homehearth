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
    -- Useful to remove any "../folder/file.h" in visual studio.
    -- Note: specify the path relative to the Premake file.
    includedirs {
		"src",
        "../ThirdParty/DirectXTK/include/",
        "../ThirdParty/DirectXTK/lib/",
        "../ThirdParty/imGUI/",
        "../ThirdParty/stb_image/",
		 "../ThirdParty/networking/",
         "../ThirdParty/entt/"
    }


    -- Define filter(s) inside project: [path + NameOfFilter] { files }.
    -- Note: create your files in 'src' folder on disk, then add them to a filter below (**filename).
    -- Else: specify the path relative to the this premake file.
    vpaths {
        ["src/Core"] = { "**EnginePCH.*" },
            ["src/Core/Engine"] = { "**Engine.*", "**Scene.*", "**EventTypes.*" },
            ["src/Core/Input"] = { "**InputSystem.*" },
            ["src/Core/Window"] = { "**Window.*" },
            ["src/Core/Utility"] = { "**Logger.*", "**multi_thread_manager.*" },

        ["src/Graphics"] = { "**Buffers.*", "**NYI.*" },
            ["src/Graphics/Renderer"] = {"**Renderer.*"},
            ["src/Graphics/D3D11"] = { "**D3D11Core.*" },
			["src/Graphics/D2D1"] = { "**D2D1Core.*" },
            ["src/Graphics/Window"] = { "**Window.*" },
			
			["src/Network"] = { "**Client.*" },

        ["src/Resources"] = { "**ResourceManager.*", "**GResource.*", "**RTexture.*" },
            ["src/Resources/Shaders"] = { "**.hhlsl", "**.hlsli" },

        ["src/Audio"] = {  },
        ["src/Physics"] = {  },
        ["src/Network"] = { "**Client.*" },
        ["src/Animation"] = {  },
        ["src/ESC"] = {  }, 
        ["src/AI"] = {  }     
    }


    links{
        "d3d11",
        "d2d1",
		"dwrite"
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
        libdirs{"../ThirdParty/DirectXTK/lib/Debug_lib/"}
        links{"DirectXTK", "DirectXTKAudioWin7"}


    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        --buildoptions "/MT"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "on"
        libdirs{"../ThirdParty/DirectXTK/lib/Release_lib/"}
        links{"DirectXTK", "DirectXTKAudioWin7"}
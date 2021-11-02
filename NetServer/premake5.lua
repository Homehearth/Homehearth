project "NetServer"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    disablewarnings{"26812"}
    linkoptions { "-IGNORE:4075", "-IGNORE:4098", "-IGNORE:4099"}
    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")
 
    
    -- Define the location of pre-compiled header.
    pchheader "NetServerPCH.h"
	pchsource "src/NetServerPCH.cpp"

    
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
		"../Game/src/",
        "../Engine/src",
        "../ThirdParty/DirectXTK/include/",
        "../ThirdParty/imGUI/",
        "../ThirdParty/stb_image/",
        "../ThirdParty/networking/",
        "../ThirdParty/entt/",
		"../ThirdParty/assimp/include/"
    }


    -- Define filter(s) inside project: [path + NameOfFilter] { files }.
    -- Note: create your files in 'src' folder on disk, then add them to a filter below (**filename).
    -- Else: specify the path relative to the this premake file.
    vpaths {
        ["src/Core"] = { "**Window.*", "**Logger.*", "**NetServer.*", "**InputSystem.*", "**Server.*", "**multi_thread_manager.*" }   
    }


    links{
        "ImGui",
        "Engine",
        "d3d11",
        "d2d1",
		"dwrite",
        "dxgi",
        "DirectXTK",
		"assimp-vc142-mt.lib",
    }

    libdirs{"../ThirdParty/imGUI/"}

    -- Define a macro/symbol which applies for the Windows system.
    filter {"system:windows"}
        defines {
            "WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }


    -- Define a macro/symbol which applies only to debug builds.
    filter {"configurations:Debug"}
        staticruntime "on"
        runtime "Debug"
        defines{"_DEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Debug_lib/",
            "../Engine/build/bin/Debug-windows-x86_64/Engine",
			"../ThirdParty/assimp/lib/"
        }


    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        staticruntime "on"
        runtime "Release"
        defines{"SERVER_LOG", "NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Release_lib/",
            "../Engine/build/bin/Release-windows-x86_64/Engine",
			"../ThirdParty/assimp/lib/"
        }
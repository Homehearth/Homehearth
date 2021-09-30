project "NetServer"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++17"
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
		 "../ThirdParty/networking/"
    }


    -- Define filter(s) inside project: [path + NameOfFilter] { files }.
    -- Note: create your files in 'src' folder on disk, then add them to a filter below (**filename).
    -- Else: specify the path relative to the this premake file.
    vpaths {
        ["src/Core"] = { "**Window.*", "**Logger.*", "**NetServer.*", "**InputSystem.*", "**Server.*", "**multi_thread_manager.*" }   
    }


    links{

    }


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


    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        staticruntime "on"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "on"
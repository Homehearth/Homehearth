project "Game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir("build/bin/" .. outputdir .. "/%{prj.name}")
    objdir("build/bin-int/" .. outputdir .. "/%{prj.name}")


    -- Define the location(s) of files.
    files {
        "src/**.h",
		"src/**.cpp",
    }


    -- Define any includes of folder(s).
    -- Useful to remove any "../folder/file.h" in code.
    includedirs {
        "src",
        "../Engine/src",
        "../ThirdParty/DirectXTK/include/",
        "../ThirdParty/imGUI/",
        "../ThirdParty/stb_image/",
        "../ThirdParty/networking/",
        "../ThirdParty/entt/"
    }

    vpaths {
        ["src/Game"] = { "**Game.*" },
            ["src/Game/Scenes"] = { "**DemoScene.*" }
    }

    links {
        "Engine",
        "d3d11",
        "d2d1",
		"dwrite",
        "dxgi",
        "DirectXTK",
        "DirectXTKAudioWin7"
    }

    filter {"system:windows"}
        defines {
            "WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter {"configurations:Debug"}
        buildoptions "/MTd"
        runtime "Debug"
        defines{"_DEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Debug_lib/",
            "../Engine/build/bin/Debug-windows-x86_64/Engine"
        }


    filter {"configurations:Release"}
        buildoptions "/MT"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        optimize "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Release_lib/",
            "../Engine/build/bin/Release-windows-x86_64/Engine"
        }

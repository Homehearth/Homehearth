project "Game"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    disablewarnings{"26812"}
    linkoptions { "-IGNORE:4075", "-IGNORE:4098", "-IGNORE:4099"}
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
        "../ThirdParty/entt/",
        "../ThirdParty/irrKlang/include/",
		"../ThirdParty/assimp/include/"
    }

    vpaths {
        ["src/Game"] = { "**Game.*" },
            ["src/Game/Scenes"] = { "**SceneHelper.*" }
    }

    links {
        "Engine",
        "d3d11",
        "d2d1",
		"dwrite",
        "dxgi",
        "DirectXTK",
		"assimp-vc142-mt.lib",
        "irrKlang.lib"
    }

    filter {"system:windows"}
        defines {
            "WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }
		
        postbuildcommands {
            "{COPYFILE} %{wks.location}Assets/Dlls/assimp-vc142-mt.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/assimp-vc142-mt.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/ikpFlac.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/ikpFlac.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/irrKlang.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/irrKlang.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/ikpMP3.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/ikpMP3.dll",
        
            "{COPYFILE} %{wks.location}Assets/Dlls/assimp-vc142-mt.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/assimp-vc142-mt.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/ikpFlac.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/ikpFlac.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/irrKlang.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/irrKlang.dll",
            "{COPYFILE} %{wks.location}Assets/Dlls/ikpMP3.dll %{wks.location}Game/build/bin/" .. outputdir .. "/Game/ikpMP3.dll"
        }

    filter {"configurations:Debug"}
        kind "ConsoleApp"
        staticruntime "on"
        runtime "Debug"
        defines{"_DEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Debug_lib/",
            "../Engine/build/bin/Debug-windows-x86_64/Engine",
			"../ThirdParty/assimp/lib/",
            "../ThirdParty/irrKlang/lib/Winx64-visualStudio/"
		}

    filter {"configurations:Release"}
        kind "WindowedApp"
        staticruntime "on"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        optimize "on"
        libdirs{
            "../ThirdParty/DirectXTK/lib/Release_lib/",
            "../Engine/build/bin/Release-windows-x86_64/Engine",
			"../ThirdParty/assimp/lib/",
            "../ThirdParty/irrKlang/lib/Winx64-visualStudio/"
		}

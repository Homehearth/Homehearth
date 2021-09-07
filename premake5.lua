-- Solution configurations file.
workspace "Workspace"
    system "Windows"
    architecture "x86_64"

    configurations {
        "Debug",
        "Release"
    }

    -- Set the startup project.
    startproject "Engine"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Subprojects.
include "Game"
include "NetServer"
include "Engine"


-- Clean.
newaction {
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing binaries")
        os.rmdir("Engine/build/bin")
        os.rmdir("Game/build/bin")
        os.rmdir("NetServer/build/bin")

        print("Removing intermediate binaries")
        os.rmdir("Engine/build/bin-int")
        os.rmdir("Game/build/bin-int")
        os.rmdir("NetServer/build/bin-int")

        print("Removing project files")
        os.rmdir("./.vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Done")
    end
}
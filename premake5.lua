-- Build.
workspace "Workspace"
    system "Windows"
    architecture "x86_64"

    configurations {
        "Debug",
        "Release"
    }

    startproject "Engine"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Game"
include "Engine"


-- Clean.
newaction {
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing binaries")
        os.rmdir("Engine/build/bin")
        os.rmdir("Game/build/bin")
        print("Removing intermediate binaries")
        os.rmdir("Engine/build/bin-int")
        os.rmdir("Game/build/bin-int")
        print("Removing project files")
        os.rmdir("./.vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Done")
    end
}
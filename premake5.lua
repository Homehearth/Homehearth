-- Build.
workspace "Workspace"
    architecture "x64"

    configurations {
        "Debug",
        "Release"
    }

    startproject "Game"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Game"
include "Engine"


-- Clean.
newaction {
    trigger = "clean",
    description = "Remove all binaries and intermediate binaries, and vs files.",
    execute = function()
        print("Removing binaries")
        os.rmdir("./build/bin")
        print("Removing intermediate binaries")
        os.rmdir("./build/bin-int")
        print("Removing project files")
        os.rmdir("./.vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Done")
    end
}
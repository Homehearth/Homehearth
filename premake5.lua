-- Build.
workspace "Workspace"
    architecture "x64"

    configurations {
        "Debug",
        "Release"
    }

    startproject "Client"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Engine"
include "Client"
include "Server"
include "Tests"
include "Editor"


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
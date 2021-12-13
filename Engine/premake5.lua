project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    disablewarnings{"26812"}
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
        "**.hlsl", 
        "**.hlsli",
        "*.mp3"
    }


    -- Define any includes of folder(s).
    -- Useful to remove any "../folder/file.h" in visual studio.
    -- Note: specify the path relative to the Premake file.
    includedirs {
		"src",
        "../ThirdParty/DirectXTK/include/",
        "../ThirdParty/imGUI/",
        "../ThirdParty/stb_image/",
		"../ThirdParty/networking/",
        "../ThirdParty/entt/",
        "../ThirdParty/irrKlang/include/",
        "../ThirdParty/assimp/include/",
        "../Assets/Sounds/"
    }


    -- Define filter(s) inside project: [path + NameOfFilter] { files }.
    -- Note: create your files in 'src' folder on disk, then add them to a filter below (**filename).
    -- Else: specify the path relative to the this premake file.
    vpaths {
        ["src/Engine"] = { "**EnginePCH.*", "**CommonStructures.*" },
            ["src/Engine/AI"] = { "**AISystem*", "**PathFinderManager.*", "**AIBehaviors.*" },  
                ["src/Engine/AI/BehaviorTree"] = {"**BT.*"},
                ["src/Engine/AI/BehaviorTree/CustomNodes"] = {"**CBT.*"},
            ["src/Engine/Sound"] = { "**SoundHandler.*" },
            ["src/Engine/Core"] = { "**Engine.*", "**EventTypes.*", "**Window.*" },
            ["src/Engine/ECS"] = { "**Components.*",  "**Entity.*", "**Tags*" }, 
            ["src/Engine/GamePlay"] = { "**GridSystem.*", "**CollisionSystem.*", "**CombatSystem.*", "**Systems.*", "**SpreeHandler.*" },

             ["src/Engine/Graphics/Renderer"] = {"**Renderer.*", "**PipelineManager.*", "**DoubleBuffer.*"},
                ["src/Engine/Graphics/Renderer/RenderPass"] = {"**Pass.*"},
			["src/Engine/Graphics/Renderer2D"] = {"**Handler2D.*", "**Element2D.*", "**Collection2D.*"},
            ["src/Engine/Graphics/D3D11"] = { "**D3D11Core.*" },
			["src/Engine/Graphics/D2D1"] = { "**D2D1Core.*" },

            ["src/Engine/Input"] = { "**InputSystem.*" },
			
			["src/Engine/Options"] = { "**OptionSystem.*", "**FileHandler.*" },
			
            ["src/Engine/Resources"] = { "**ResourceManager.*", "**GResource.*", "**RModel.*", "**RTexture.*", "**RMaterial.*"},
                ["src/Engine/Resources/Shaders"] = { "**.hlsl", "**.hlsli", "**Shader.*" },
		        ["src/Engine/Resources/Animation"] = {"**RAnimation.*", "**RAnimator.*", "**AnimStructures.*"},

            ["src/Engine/Scene"] = {"**Scene.*", "**Camera.*", "**Lights.*", "**ModelIdentifier.*", "**ParticleSystem.*" , "**Skybox.*" },
            ["src/Engine/Thread"] = { "**multi_thread_manager.*", "**ThreadSyncer.*", "**RenderThreadHandler.*"},
            
		    ["src/Engine/UI Elements"] = {"**Canvas.*", "**Picture.*", "**Border.*", "**Button.*", "**Text.*", "**CText.*", "**TextField.*", "**Slider.*", "**Healthbar.*", "**Scroller.*", "**MoneyUI.*", "**AbilityUI.*", "**ShopUI.*", "**MenuUI.*"},
            ["src/Engine/Utility"] = { "**Timer.*", "**Profiler.*", "**Logger.*", "**Stats.*", "**QuadTree.*" },
        
            ["src/Engine/Audio"] = {  },
            ["src/Engine/Physics"] = {  },

        ["src/Network"] = { "**Client.*" },
			["src/Network/Prediction"] = {"**Predictor.*"}
    }


    links{
        "dxgi", -- links d3d11 d2d1 dwrite
        "DirectXTK",
        "ImGui",
        "irrKlang.lib"
    }

    libdirs{
        "../ThirdParty/imGUI/",
        "../ThirdParty/irrKlang/lib/Winx64-visualStudio/"
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
        libdirs{"../ThirdParty/DirectXTK/lib/Debug_lib/"}



    -- Define a macro/symbol which applies only to release builds.
    filter {"configurations:Release"}
        staticruntime "on"
        runtime "Release"
        defines{"NDEBUG", "_UNICODE", "UNICODE"}
        symbols "on"
        optimize "Full"
        libdirs{"../ThirdParty/DirectXTK/lib/Release_lib/"}




    filter "*"
        --local ws = "$(ProjectDir)%%(Filename).cso"
        local ws = "../Game/%%(Filename).cso"
        
        files("*.hlsl")
            shadermodel("5.0")
            shaderobjectfileoutput(ws)

        filter("files:**_vs.hlsl")
            shadertype("Vertex")

        filter("files:**_ps.hlsl")
            shadertype("Pixel")
            
        filter("files:**_gs.hlsl")
            shadertype("Geometry")
           
        filter("files:**_cs.hlsl")
            shadertype("Compute")
			
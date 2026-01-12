 -- 根据不同的生成器设置项目目录
if _ACTION == "vs2022" then
    projectdir = "Projects/Windows/Visual Studio 2022"
elseif _ACTION == "vs2019" then
    projectdir = "Projects/Windows/Visual Studio 2019"
elseif _ACTION == "xcode4" then
    projectdir = "Projects/macOS/Xcode"
elseif _ACTION == "gmake2" then
    if os.host() == "linux" then
        projectdir = "Projects/Linux/Makefiles"
    elseif os.host() == "macosx" then
        projectdir = "Projects/macOS/Makefiles"
    else
        projectdir = "Projects/Unix/Makefiles"
    end
else
    -- 默认情况
    projectdir = "Projects/Generated"
end

workspace "TinyEngine"
	architecture "x64"
	startproject "EngineCore"
	location (projectdir)

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


-- Include directories relative to root folder (solution directory)
IncludeDir = {}
-- IncludeDir["GLFW"] = "ThirdParty/Runtime/Graphics/GLFW/include"
-- IncludeDir["Glad"] = "ThirdParty/Runtime/Graphics/GLAD/include"
-- IncludeDir["ImGui"] = "ThirdParty/Editor/imgui"
-- IncludeDir["glm"] = "ThirdParty/Runtime/Core/glm"
-- IncludeDir["stb_image"] = "ThirdParty/Runtime/Asset/stb_image"
-- IncludeDir["assimp"] = "ThirdParty/Runtime/Asset/assimp/include"
-- IncludeDir["entt"] = "ThirdParty/Runtime/Core/entt/include"
-- IncludeDir["boost"] = "ThirdParty/Runtime/Core/boost"

group "Dependencies"
	-- -- This include file include the GLFW premake5.lua.
	-- include "ThirdParty/Runtime/Graphics/GLFW"
	-- include "ThirdParty/Runtime/Graphics/GLAD"
	-- include "ThirdParty/Editor/imgui"
	-- include "ThirdParty/Runtime/Asset/assimp"
	-- include "ThirdParty/Runtime/Core/boost"
group ""



project "EngineCore"
	location (projectdir .. "/EngineCore")
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir (projectdir .. "/bin/" .. outputdir .. "/%{prj.name}")
	objdir (projectdir .. "/bin-int/" .. outputdir .. "/%{prj.name}")
	-- defines { "RENDER_API_OPENGL", "TRACE" }
	--defines { "RENDER_API_DIRECTX12", "TRACE" }
	pchheader "PreCompiledHeader.h"
	pchsource "Runtime/PreCompiledHeader.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"Runtime/**.cpp",
		"Runtime/**.h",
		"Editor/**.h",
		"Editor/**.cpp",
		-- "ThirdParty/Runtime/Asset/stb_image/**.h",
		-- "ThirdParty/Runtime/Asset/stb_image/**.cpp",
		-- "ThirdParty/Runtime/Core/glm/glm/**.hpp",
		-- "ThirdParty/Runtime/Core/glm/glm/**.inl",
	}

    filter "files:Runtime/External/**"
        flags { "NoPCH" }
    filter {}  -- 重置filter
	
	includedirs
	{
		"%{prj.name}/src",
		"Runtime/",
		"Runtime/External/Imgui",
		"Runtime/External/stb_image",
		"Runtime/External/nlohmann",
		"Editor/",
		"Vendor/libs",
		"Vendor/Assimp/assimp/include",
		"Vendor/Assimp/assimp/contrib/rapidjson/include"
		-- "ThirdParty/Runtime/Core/spdlog/include",
		-- "%{IncludeDir.GLFW}",
		-- "%{IncludeDir.Glad}",
		-- "%{IncludeDir.ImGui}",
		-- "%{IncludeDir.glm}",
		-- "%{IncludeDir.stb_image}",
		-- "%{IncludeDir.assimp}",
		-- "%{IncludeDir.entt}",
		-- "%{IncludeDir.boost}"
	}

	links 
	{ 
		-- "GLFW",
		-- "Glad",
		-- "ImGui",
		-- "opengl32.lib",
	}

  	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TINYGLTF_USE_RAPIDJSON",
			-- "HZ_PLATFORM_WINDOWS",
			-- "HZ_BUILD_DLL",
			-- "GLFW_INCLUDE_NONE",
			-- "RENDER_API_DIRECTX12",
			-- "NOMINMAX"
		}
		buildoptions { "/source-charset:utf-8", "/execution-charset:utf-8" }

	filter "configurations:Debug"
		defines {
			"DEBUG_MODE",
			'ENGINE_ROOT_PATH="' .. path.getabsolute(".") .. '/"',
			'ASSETS_PATH="' .. path.getabsolute("./Assets") .. '/"'			
		}
		runtime "Debug"
		staticruntime "on"
		symbols "on"
		links { "Vendor/libs/assimpd" }
   		debugdir "%{wks.location}/../.."

	filter "configurations:Release"
		defines { 
			"RELEASE_MODE",
			'ENGINE_ROOT_PATH="' .. path.getabsolute(".") .. '/"',
			'ASSETS_PATH="' .. path.getabsolute("./Assets") .. '/"'
		}
		runtime "Release"
		optimize "on"
		links { "Vendor/libs/assimp" }
		debugdir "%{wks.location}/../.."

	filter "configurations:Dist"
		defines { 
			"DIST_MODE",
			'ENGINE_ROOT_PATH=""',  -- 发布版本使用相对于exe的路径
			'ASSETS_PATH="Assets/"'
		}
		runtime "Release"
		optimize "on"
		links { "Vendor/libs/assimp" }
group "Core"
project "CoreLib"
  kind "StaticLib"
  language "C++"
  cppdialect "C++20"

  targetdir ("%{wks.location}/bin/" .. bldCfg .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. bldCfg .. "/%{prj.name}")

  files { "src/**.cpp", "include/**.h" }

  includedirs {
    "include/",
    "%{libDir}/glfw/include"
  }

  libdirs {

  }

  links {
    "glfw"
  }

CORE_INC_DIR = "$(SolutionDir)/Core/include/"
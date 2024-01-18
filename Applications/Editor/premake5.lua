group "Editor"
project "Editor"
  kind "ConsoleApp"
  language "C++"
  cppdialect "C++20"

  targetdir ("%{wks.location}/bin/" .. bldCfg .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. bldCfg .. "/%{prj.name}")

  files { "src/**.cpp", "include/**.h" }

  includedirs {
    "include/",
    CORE_INC_DIR
  }

  libdirs {

  }

  links {
    "Editor"
  }
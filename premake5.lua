workspace "PurrfectEngine"
  configurations { "Debug", "Release", "Dist" }
  startproject "Editor"
  architecture "x86_64"
  
  flags {
		"MultiProcessorCompile"
	}

bldCfg = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
libDir = "%{wks.location}/Libraries"

include "Libraries"
include "Core"
include "Applications"
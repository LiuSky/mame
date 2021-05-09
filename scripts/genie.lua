-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team
STANDALONE = false

-- Big project specific
premake.make.makefile_ignore = true
premake._checkgenerate = false

newoption {
	trigger = 'build-dir',
	description = 'Build directory name',
}

premake.check_paths = true
premake.make.override = { "TARGET" }

premake.xcode.parameters = { 'CLANG_CXX_LANGUAGE_STANDARD = "c++17"', 'CLANG_CXX_LIBRARY = "libc++"' }

MAME_DIR = (path.getabsolute("..") .. "/")
--MAME_DIR = string.gsub(MAME_DIR, "(%s)", "\\%1")
local MAME_BUILD_DIR = (MAME_DIR .. _OPTIONS["build-dir"] .. "/")
local naclToolchain = ""

newoption {
	trigger = "precompile",
	description = "Precompiled headers generation.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

function backtick(cmd)
	result = string.gsub(string.gsub(os.outputof(cmd), "\r?\n$", ""), " $", "")
	return result
end

function str_to_version(str)
	local val = 0
	if (str == nil or str == '') then
		return val
	end
	local cnt = 10000
	for word in string.gmatch(str, '([^.]+)') do
		if(tonumber(word) == nil) then
			return val
		end
		val = val + tonumber(word) * cnt
		cnt = cnt / 100
	end
	return val
end

function findfunction(x)
	assert(type(x) == "string")
	local f=_G
	for v in x:gmatch("[^%.]+") do
	if type(f) ~= "table" then
		return nil, "looking for '"..v.."' expected table, not "..type(f)
	end
	f=f[v]
	end
	if type(f) == "function" then
	return f
	else
	return nil, "expected function, not "..type(f)
	end
end

function layoutbuildtask(_folder, _name)
	return { MAME_DIR .. "src/".._folder.."/".. _name ..".lay" ,    GEN_DIR .. _folder .. "/".._name..".lh",
		{  MAME_DIR .. "scripts/build/complay.py" }, {"@echo Compressing src/".._folder.."/".._name..".lay...",    PYTHON .. " $(1) $(<) $(@) layout_".._name }};
end

function precompiledheaders()
	if _OPTIONS["precompile"]==nil or (_OPTIONS["precompile"]~=nil and _OPTIONS["precompile"]=="1") then
		configuration { "not xcode4" }
			pchheader("emu.h")
		configuration { }
	end
end

function precompiledheaders_novs()
	precompiledheaders()
	if string.sub(_ACTION,1,4) == "vs20" then
		--print("Disabling pch for Visual Studio")
		flags {
			"NoPCH"
		}
	end
end

function addprojectflags()
	local version = str_to_version(_OPTIONS["gcc_version"])
	if _OPTIONS["gcc"]~=nil and string.find(_OPTIONS["gcc"], "gcc") then
		buildoptions_cpp {
			"-Wsuggest-override",
			"-flifetime-dse=1",
		}
	end
end

function opt_tool(hash, entry)
   if _OPTIONS["with-tools"] then
	  hash[entry] = true
	  return true
   end
   return hash[entry]
end

CPUS = {}
SOUNDS  = {}
MACHINES  = {}
VIDEOS = {}
BUSES  = {}
FORMATS  = {}

newoption {
	trigger = "with-tools",
	description = "Enable building tools.",
}

newoption {
	trigger = "with-tests",
	description = "Enable building tests.",
}

newoption {
	trigger = "with-benchmarks",
	description = "Enable building benchmarks.",
}

newoption {
	trigger = "osd",
	description = "Choose OSD layer implementation",
}

newoption {
	trigger = "targetos",
	description = "Choose target OS",
	allowed = {
		{ "android",       "Android"                },
		{ "asmjs",         "Emscripten/asm.js"      },
		{ "freebsd",       "FreeBSD"                },
		{ "netbsd",        "NetBSD"                 },
		{ "openbsd",       "OpenBSD"                },
		{ "pnacl",         "Native Client - PNaCl"  },
		{ "linux",         "Linux"                  },
		{ "ios",           "iOS"                    },
		{ "macosx",        "OSX"                    },
		{ "windows",       "Windows"                },
		{ "haiku",         "Haiku"                  },
		{ "solaris",       "Solaris SunOS"          },
		{ "steamlink",     "Steam Link"             },
		{ "rpi",           "Raspberry Pi"           },
		{ "ci20",          "Creator-Ci20"           },
	},
}

newoption {
	trigger = 'with-bundled-sdl2',
	description = 'Build bundled SDL2 library',
}

newoption {
	trigger = "distro",
	description = "Choose distribution",
	allowed = {
		{ "generic",           "generic"            },
		{ "debian-stable",     "debian-stable"      },
		{ "ubuntu-intrepid",   "ubuntu-intrepid"    },
	},
}

newoption {
	trigger = "target",
	description = "Building target",
}

newoption {
	trigger = "subtarget",
	description = "Building subtarget",
}

newoption {
	trigger = "gcc_version",
	description = "GCC compiler version",
}

newoption {
	trigger = "CC",
	description = "CC replacement",
}

newoption {
	trigger = "CXX",
	description = "CXX replacement",
}

newoption {
	trigger = "LD",
	description = "LD replacement",
}

newoption {
	trigger = "TOOLCHAIN",
	description = "Toolchain prefix"
}

newoption {
	trigger = "PROFILE",
	description = "Enable profiling.",
}

newoption {
	trigger = "SYMBOLS",
	description = "Enable symbols.",
}

newoption {
	trigger = "SYMLEVEL",
	description = "Symbols level.",
}

newoption {
	trigger = "PROFILER",
	description = "Include the internal profiler.",
}

newoption {
	trigger = "OPTIMIZE",
	description = "Optimization level.",
}

newoption {
	trigger = "ARCHOPTS",
	description = "Additional options for target C/C++/Objective-C/Objective-C++ compilers and linker.",
}

newoption {
	trigger = "ARCHOPTS_C",
	description = "Additional options for target C++ compiler.",
}

newoption {
	trigger = "ARCHOPTS_CXX",
	description = "Additional options for target C++ compiler.",
}

newoption {
	trigger = "ARCHOPTS_OBJC",
	description = "Additional options for target Objective-C compiler.",
}

newoption {
	trigger = "ARCHOPTS_OBJCXX",
	description = "Additional options for target Objective-C++ compiler.",
}

newoption {
	trigger = "OPT_FLAGS",
	description = "OPT_FLAGS.",
}

newoption {
	trigger = "LDOPTS",
	description = "Additional linker options",
}

newoption {
	trigger = "MAP",
	description = "Generate a link map.",
}

newoption {
	trigger = "NOASM",
	description = "Disable implementations based on assembler code",
	allowed = {
		{ "0",  "Enable assembler code"   },
		{ "1",  "Disable assembler code"  },
	},
}

newoption {
	trigger = "BIGENDIAN",
	description = "Build for big endian target",
	allowed = {
		{ "0",  "Little endian target"   },
		{ "1",  "Big endian target"  },
	},
}

newoption {
	trigger = "FORCE_DRC_C_BACKEND",
	description = "Force DRC C backend.",
}

newoption {
	trigger = "NOWERROR",
	description = "NOWERROR",
}

newoption {
	trigger = "DEPRECATED",
	description = "Generate deprecation warnings during compilation.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "LTO",
	description = "Clang link time optimization.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "SSE2",
	description = "SSE2 optimized code and SSE2 code generation.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "SSE3",
	description = "SSE3 optimized code and SSE3 code generation.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "OPENMP",
	description = "OpenMP optimized code.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "SEPARATE_BIN",
	description = "Use separate bin folders.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "PYTHON_EXECUTABLE",
	description = "Python executable.",
}

newoption {
	trigger = "SHADOW_CHECK",
	description = "Shadow checks.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}

newoption {
	trigger = "STRIP_SYMBOLS",
	description = "Symbols stripping.",
	allowed = {
		{ "0",   "Disabled"     },
		{ "1",   "Enabled"      },
	}
}


newoption {
	trigger = "SHLIB",
	description = "Generate shared libs.",
	allowed = {
		{ "0",   "Static libs"  },
		{ "1",   "Shared libs"  },
	}
}

newoption {
	trigger = "SOURCES",
	description = "List of sources to compile.",
}

newoption {
	trigger = "PLATFORM",
	description = "Target machine platform (x86,arm,...)",
}

newoption {
	trigger = "DEBUG_DIR",
	description = "Default directory for debugger.",
}

newoption {
	trigger = "DEBUG_ARGS",
	description = "Arguments for running debug build.",
}

newoption {
	trigger = "WEBASSEMBLY",
	description = "Produce WebAssembly output when building with Emscripten.",
}

newoption {
	trigger = "SANITIZE",
	description = "Specifies the santizer(s) to use."
}

newoption {
	trigger = "PROJECT",
	description = "Select projects to be built. Will look into project folder for files.",
}

dofile ("extlib.lua")

if _OPTIONS["SHLIB"]=="1" then
	LIBTYPE = "SharedLib"
else
	LIBTYPE = "StaticLib"
end

PYTHON = "python"

if _OPTIONS["PYTHON_EXECUTABLE"]~=nil then
	PYTHON = _OPTIONS["PYTHON_EXECUTABLE"]
end

if not _OPTIONS["BIGENDIAN"] then
	_OPTIONS["BIGENDIAN"] = "0"
end

if _OPTIONS["NOASM"]=="1" and not _OPTIONS["FORCE_DRC_C_BACKEND"] then
	_OPTIONS["FORCE_DRC_C_BACKEND"] = "1"
end

if(_OPTIONS["TOOLCHAIN"] == nil) then
	_OPTIONS['TOOLCHAIN'] = ""
end

GEN_DIR = MAME_BUILD_DIR .. "generated/"

if (_OPTIONS["target"] == nil) then return false end
if (_OPTIONS["subtarget"] == nil) then return false end

if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
	solution (_OPTIONS["target"])
else
	if (_OPTIONS["subtarget"]=="mess") then
		solution (_OPTIONS["subtarget"])
	else
		solution (_OPTIONS["target"] .. _OPTIONS["subtarget"])
	end
end


configurations {
	"Debug",
	"Release",
}

if _ACTION == "xcode4" then
	platforms {
		"x64",
	}
else
	platforms {
		"x32",
		"x64",
		"Native", -- for targets where bitness is not specified
	}
end

language "C++"

flags {
	"StaticRuntime",
}

configuration { "vs20*" }
	buildoptions {
		"/bigobj",
	}
	buildoptions_cpp {
		"/std:c++17",
	}
	flags {
		"ExtraWarnings",
	}
	if not _OPTIONS["NOWERROR"] then
		flags{
			"FatalWarnings",
		}
	end


configuration { "Debug", "vs20*" }
	flags {
		"Symbols",
		"NoMultiProcessorCompilation",
	}

configuration { "Release", "vs20*" }
	flags {
		"Optimize",
		"NoEditAndContinue",
		"NoIncrementalLink",
	}
	if _OPTIONS["SYMBOLS"] then
		flags {
			"Symbols",
		}
	end

configuration { "vsllvm" }
	buildoptions {
		"/bigobj",
	}
	flags {
		"NoPCH",
		"ExtraWarnings",
	}
	if not _OPTIONS["NOWERROR"] then
		flags{
			"FatalWarnings",
		}
	end


configuration { "Debug", "vsllvm" }
	flags {
		"Symbols",
		"NoMultiProcessorCompilation",
	}

configuration { "Release", "vsllvm" }
	flags {
		"Optimize",
		"NoEditAndContinue",
		"NoIncrementalLink",
	}

-- Force VS2015/17 targets to use bundled SDL2
if string.sub(_ACTION,1,4) == "vs20" and _OPTIONS["osd"]=="sdl" then
	if _OPTIONS["with-bundled-sdl2"]==nil then
		_OPTIONS["with-bundled-sdl2"] = "1"
	end
end
-- Build SDL2 for Android
if _OPTIONS["targetos"] == "android" then
	_OPTIONS["with-bundled-sdl2"] = "1"
end

configuration {}

if _OPTIONS["osd"] == "uwp" then
	windowstargetplatformversion("10.0.14393.0")
	windowstargetplatformminversion("10.0.14393.0")
	premake._filelevelconfig = true
end

msgcompile ("Compiling $(subst ../,,$<)...")

msgcompile_objc ("Objective-C compiling $(subst ../,,$<)...")

msgresource ("Compiling resources $(subst ../,,$<)...")

msglinking ("Linking $(notdir $@)...")

msgarchiving ("Archiving $(notdir $@)...")

msgprecompile ("Precompiling $(subst ../,,$<)...")

messageskip { "SkipCreatingMessage", "SkipBuildingMessage", "SkipCleaningMessage" }

if (_OPTIONS["PROJECT"] ~= nil) then
	PROJECT_DIR = path.join(path.getabsolute(".."),"projects",_OPTIONS["PROJECT"]) .. "/"
	if (not os.isfile(path.join("..", "projects", _OPTIONS["PROJECT"], "scripts", "target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))) then
		error("File definition for TARGET=" .. _OPTIONS["target"] .. " SUBTARGET=" .. _OPTIONS["subtarget"] .. " does not exist")
	end
	dofile (path.join(".." ,"projects", _OPTIONS["PROJECT"], "scripts", "target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))
end
if (_OPTIONS["SOURCES"] == nil and _OPTIONS["PROJECT"] == nil) then
	if (not os.isfile(path.join("target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))) then
		error("File definition for TARGET=" .. _OPTIONS["target"] .. " SUBTARGET=" .. _OPTIONS["subtarget"] .. " does not exist")
	end
	dofile (path.join("target", _OPTIONS["target"],_OPTIONS["subtarget"] .. ".lua"))
end

configuration { "gmake or ninja" }
	flags {
		"SingleOutputDir",
	}

dofile ("toolchain.lua")

if _OPTIONS["targetos"]=="windows" then
	configuration { "x64" }
		defines {
			"X64_WINDOWS_ABI",
		}
	configuration { }
end

-- Avoid error when invoking genie --help.
if (_ACTION == nil) then return false end

-- define PTR64 if we are a 64-bit target
configuration { "x64 or android-*64"}
	defines { "PTR64=1" }

-- define MAME_DEBUG if we are a debugging build
configuration { "Debug" }
	defines {
		"MAME_DEBUG",
		"MAME_PROFILER",
--      "BGFX_CONFIG_DEBUG=1",
	}

configuration { }

if _OPTIONS["PROFILER"]=="1" then
	defines{
		"MAME_PROFILER", -- define MAME_PROFILER if we are a profiling build
	}
end

configuration { "Release" }
	defines {
		"NDEBUG",
	}

configuration { }

-- CR/LF setup: use on win32, CR only on everything else
if _OPTIONS["targetos"]=="windows" then
	defines {
		"CRLF=3",
	}
else
	defines {
		"CRLF=2",
	}
end


if _OPTIONS["BIGENDIAN"]=="1" then
	if _OPTIONS["targetos"]=="macosx" then
		defines {
			"OSX_PPC",
		}
		buildoptions {
			"-Wno-unused-label",
			"-flax-vector-conversions",
		}
		if _OPTIONS["SYMBOLS"] then
			buildoptions {
				"-mlong-branch",
			}
		end
		configuration { "x64" }
			buildoptions {
				"-arch ppc64",
			}
			linkoptions {
				"-arch ppc64",
			}
		configuration { "x32" }
			buildoptions {
				"-arch ppc",
			}
			linkoptions {
				"-arch ppc",
			}
		configuration { }
	end
else
	defines {
		"LSB_FIRST",
	}
	if _OPTIONS["targetos"]=="macosx" and not (_OPTIONS["ARCHOPTS"] or ""):find("-arch") then
		configuration { "arm64" }
			buildoptions {
				"-arch arm64",
			}
			linkoptions {
				"-arch arm64",
			}
		configuration { "x64", "not arm64" }
			buildoptions {
				"-arch x86_64",
			}
			linkoptions {
				"-arch x86_64",
			}
		configuration { "x32", "not arm64" }
			buildoptions {
				"-arch i386",
			}
			linkoptions {
				"-arch i386",
			}
		configuration { }
	end
end

if _OPTIONS["with-system-jpeg"]~=nil then
	defines {
		"XMD_H",
	}
end

if not _OPTIONS["with-system-flac"]~=nil then
	defines {
		"FLAC__NO_DLL",
	}
end

if not _OPTIONS["with-system-pugixml"] then
	defines {
		"PUGIXML_HEADER_ONLY",
	}
else
	links {
		ext_lib("pugixml"),
	}
end

if _OPTIONS["NOASM"]=="1" then
	defines {
		"MAME_NOASM"
	}
end

if not _OPTIONS["FORCE_DRC_C_BACKEND"] then
	if _OPTIONS["BIGENDIAN"]~="1" then
		configuration { "x64" }
			defines {
				"NATIVE_DRC=drcbe_x64",
			}
		configuration { "x32" }
			defines {
				"NATIVE_DRC=drcbe_x86",
			}
		configuration {  }
	end
end

	defines {
		"LUA_COMPAT_ALL",
		"LUA_COMPAT_5_1",
		"LUA_COMPAT_5_2",
	}

	if _ACTION == "gmake" or _ACTION == "ninja" then

	--we compile C-only to C99 standard with GNU extensions

	buildoptions_c {
		"-std=gnu99",
	}

local version = str_to_version(_OPTIONS["gcc_version"])
	buildoptions_cpp {
		"-std=c++17",
	}

	buildoptions_objcpp {
		"-std=c++17",
	}
-- this speeds it up a bit by piping between the preprocessor/compiler/assembler
	if not ("pnacl" == _OPTIONS["gcc"]) then
		buildoptions {
			"-pipe",
		}
	end
-- add -g if we need symbols, and ensure we have frame pointers
if _OPTIONS["SYMBOLS"]~=nil and _OPTIONS["SYMBOLS"]~="0" then
	buildoptions {
		"-g" .. _OPTIONS["SYMLEVEL"],
		"-fno-omit-frame-pointer",
		"-fno-optimize-sibling-calls",
	}
end

--# we need to disable some additional implicit optimizations for profiling
if _OPTIONS["PROFILE"] then
	buildoptions {
		"-mno-omit-leaf-frame-pointer",
	}
end
-- add -v if we need verbose build information
if _OPTIONS["VERBOSE"] then
	buildoptions {
		"-v",
	}
end

-- only show shadow warnings when enabled
if (_OPTIONS["SHADOW_CHECK"]=="1") then
	buildoptions {
		"-Wshadow"
	}
end

-- only show deprecation warnings when enabled
if _OPTIONS["DEPRECATED"]=="0" then
	buildoptions {
		"-Wno-deprecated-declarations"
	}
end

-- add profiling information for the compiler
if _OPTIONS["PROFILE"] then
	buildoptions {
		"-pg",
	}
	linkoptions {
		"-pg",
	}
end

if _OPTIONS["SYMBOLS"]~=nil and _OPTIONS["SYMBOLS"]~="0" then
	flags {
		"Symbols",
	}
end

-- add the error warning flag
if _OPTIONS["NOWERROR"]==nil then
	buildoptions {
		"-Werror",
	}
end

-- if we are optimizing, include optimization options
if _OPTIONS["OPTIMIZE"] then
	buildoptions {
		"-O".. _OPTIONS["OPTIMIZE"],
		"-fno-strict-aliasing"
	}
	if _OPTIONS["OPT_FLAGS"] then
		buildoptions {
			_OPTIONS["OPT_FLAGS"]
		}
	end
	if _OPTIONS["LTO"]=="1" then
		buildoptions {
-- windows native mingw GCC 5.2 fails with -flto=x with x > 1. bug unfixed as of this commit
			"-flto=1",
-- if ld fails, just buy more RAM or uncomment this!
--          "-Wl,-no-keep-memory",
			"-Wl,-v",
-- silence redefine warnings from discrete.c.
			"-Wl,-allow-multiple-definition",
			"-fuse-linker-plugin",
-- these next flags allow MAME to compile in GCC 5.2. odr warnings should be fixed as LTO randomly crashes otherwise
-- some GCC 4.9.x on Windows do not have -Wodr and -flto-odr-type-merging enabled. adjust accordingly...
-- no-fat-lto-objects is faster to compile and uses less memory, but you can't mix with a non-lto .o/.a without rebuilding
			"-fno-fat-lto-objects",
			"-flto-odr-type-merging",
			"-Wodr",
			"-flto-compression-level=0", -- lto doesn't work with anything <9 on linux with < 12G RAM, much slower if <> 0
--          "-flto-report", -- if you get an error in lto after [WPA] stage, but before [LTRANS] stage, you need more memory!
--          "-fmem-report-wpa","-fmem-report","-fpre-ipa-mem-report","-fpost-ipa-mem-report","-flto-report-wpa","-fmem-report",
-- this six flag combo lets MAME compile with LTO=1 on linux with no errors and ~2% speed boost, but compile time is much longer
-- if you are going to wait on lto, you might as well enable these for GCC
--          "-fdevirtualize-at-ltrans","-fgcse-sm","-fgcse-las",
--          "-fipa-pta","-fipa-icf","-fvariable-expansion-in-unroller",
		}
-- same flags are needed by linker
		linkoptions {
			"-flto=1",
--          "-Wl,-no-keep-memory",
			"-Wl,-v",
			"-Wl,-allow-multiple-definition",
			"-fuse-linker-plugin",
			"-fno-fat-lto-objects",
			"-flto-odr-type-merging",
			"-Wodr",
			"-flto-compression-level=0", -- lto doesn't work with anything <9 on linux with < 12G RAM, much slower if <> 0
--          "-flto-report", -- if you get an error in lto after [WPA] stage, but before [LTRANS] stage, you need more memory!
--          "-fmem-report-wpa","-fmem-report","-fpre-ipa-mem-report","-fpost-ipa-mem-report","-flto-report-wpa","-fmem-report",
-- this six flag combo lets MAME compile with LTO=1 on linux with no errors and ~2% speed boost, but compile time is much longer
-- if you are going to wait on lto, you might as well enable these for GCC
--          "-fdevirtualize-at-ltrans","-fgcse-sm","-fgcse-las",
--          "-fipa-pta","-fipa-icf","-fvariable-expansion-in-unroller",

		}

	end
end

configuration { "mingw-clang" }
	buildoptions {
		"-Xclang -flto-visibility-public-std", -- workround for __imp___ link errors
		"-Wno-nonportable-include-path", -- workround for clang 9.0.0 case sensitivity bug when including GL/glext.h
	}
configuration {  }

if _OPTIONS["ARCHOPTS"] then
	buildoptions {
		_OPTIONS["ARCHOPTS"]
	}
	linkoptions {
		_OPTIONS["ARCHOPTS"]
	}
end

if _OPTIONS["ARCHOPTS_C"] then
	buildoptions_c {
		_OPTIONS["ARCHOPTS_C"]
	}
end

if _OPTIONS["ARCHOPTS_CXX"] then
	buildoptions_cpp {
		_OPTIONS["ARCHOPTS_CXX"]
	}
end

if _OPTIONS["ARCHOPTS_OBJC"] then
	buildoptions_objc {
		_OPTIONS["ARCHOPTS_OBJC"]
	}
end

if _OPTIONS["ARCHOPTS_OBJCXX"] then
	buildoptions_objcpp {
		_OPTIONS["ARCHOPTS_OBJCXX"]
	}
end

if _OPTIONS["SHLIB"] then
	buildoptions {
		"-fPIC"
	}
end

if _OPTIONS["SSE2"]=="1" then
	buildoptions {
		"-msse",
		"-msse2"
	}
end

if _OPTIONS["SSE3"]=="1" then
	buildoptions {
		"-msse",
		"-msse2",
		"-msse3"
	}
end


if _OPTIONS["OPENMP"]=="1" then
	buildoptions {
		"-fopenmp",
	}
	linkoptions {
		"-fopenmp"
	}
	defines {
		"USE_OPENMP=1",
	}

else
	buildoptions {
		"-Wno-unknown-pragmas",
	}
end

if _OPTIONS["LDOPTS"] then
	linkoptions {
		_OPTIONS["LDOPTS"]
	}
end

if _OPTIONS["MAP"] then
	if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
		linkoptions {
			"-Wl,-Map," .. "../../../../" .. _OPTIONS["target"] .. ".map"
		}
	else
		linkoptions {
			"-Wl,-Map," .. "../../../../"  .. _OPTIONS["target"] .. _OPTIONS["subtarget"] .. ".map"
		}

	end
end


-- add a basic set of warnings
	buildoptions {
		"-Wall",
		"-Wcast-align",
		"-Wformat-security",
		"-Wundef",
		"-Wwrite-strings",
		"-Wno-conversion",
		"-Wno-sign-compare",
		"-Wno-error=deprecated-declarations",
	}
-- warnings only applicable to C compiles
	buildoptions_c {
		"-Wpointer-arith",
		"-Wstrict-prototypes",
	}

if _OPTIONS["targetos"]~="freebsd" then
	buildoptions_c {
		"-Wbad-function-cast",
	}
end

-- warnings only applicable to OBJ-C compiles
	buildoptions_objcpp {
		"-Wpointer-arith",
	}

-- warnings only applicable to C++ compiles
	buildoptions_cpp {
		"-Woverloaded-virtual",
	}

if _OPTIONS["SANITIZE"] then
	buildoptions {
		"-fsanitize=".. _OPTIONS["SANITIZE"]
	}
	linkoptions {
		"-fsanitize=".. _OPTIONS["SANITIZE"]
	}
	if string.find(_OPTIONS["SANITIZE"], "address") then
		buildoptions {
			"-fsanitize-address-use-after-scope"
		}
		linkoptions {
			"-fsanitize-address-use-after-scope"
		}
	end
	if string.find(_OPTIONS["SANITIZE"], "undefined") then
		-- 'function' produces errors without delegates by design
		-- 'alignment' produces a lot of errors which we are not interested in
		buildoptions {
			"-fno-sanitize=function",
			"-fno-sanitize=alignment"
		}
		linkoptions {
			"-fno-sanitize=function",
			"-fno-sanitize=alignment"
		}
	end
end

--ifneq (,$(findstring thread,$(SANITIZE)))
--CCOMFLAGS += -fPIE
--endif



		local version = str_to_version(_OPTIONS["gcc_version"])
		if string.find(_OPTIONS["gcc"], "clang") or string.find(_OPTIONS["gcc"], "pnacl") or string.find(_OPTIONS["gcc"], "asmjs") or string.find(_OPTIONS["gcc"], "android") then
			if (version < 60000) then
				print("Clang version 6.0 or later needed")
				os.exit(-1)
			end
			buildoptions {
				"-fdiagnostics-show-note-include-stack",
				"-Wno-cast-align",
				"-Wno-constant-logical-operand",
				"-Wno-extern-c-compat",
				"-Wno-ignored-qualifiers",
				"-Wno-pragma-pack", -- clang 6.0 complains when the packing change lifetime is not contained within a header file.
				"-Wno-tautological-compare",
				"-Wno-unknown-attributes",
				"-Wno-unknown-warning-option",
				"-Wno-unused-value",
			}
			if ((version >= 100000) and (_OPTIONS["targetos"] ~= 'macosx')) or (version >= 120000) then
				buildoptions {
					"-Wno-xor-used-as-pow", -- clang 10.0 complains that expressions like 10 ^ 7 look like exponention
				}
			end
		else
			if (version < 70000) then
				print("GCC version 7.0 or later needed")
				os.exit(-1)
			end
				buildoptions_cpp {
					"-Wimplicit-fallthrough",
				}
				buildoptions_objcpp {
					"-Wimplicit-fallthrough",
				}
				buildoptions {
					"-Wno-unused-result", -- needed for fgets,fread on linux
					-- array bounds checking seems to be buggy in 4.8.1 (try it on video/stvvdp1.c and video/model1.c without -Wno-array-bounds)
					"-Wno-array-bounds",
					"-Wno-error=attributes", -- GCC fails to recognize some uses of [[maybe_unused]]
				}
			if (version >= 80000) then
				buildoptions {
					"-Wno-format-overflow", -- try machine/bfm_sc45_helper.cpp in GCC 8.0.1, among others
					"-Wno-stringop-truncation", -- ImGui again
					"-Wno-stringop-overflow",   -- formats/victor9k_dsk.cpp bugs the compiler
				}
				buildoptions_cpp {
					"-Wno-class-memaccess", -- many instances in ImGui and BGFX
				}
			end
			if (version >= 100000) then
				buildoptions {
					"-Wno-return-local-addr", -- sqlite3.c in GCC 10
				}
			end
		end
	end

if (_OPTIONS["PLATFORM"]=="alpha") then
	defines {
		"PTR64=1",
	}
end

if (_OPTIONS["PLATFORM"]=="arm") then
	buildoptions {
		"-Wno-cast-align",
	}
end

if (_OPTIONS["PLATFORM"]=="arm64") then
	buildoptions {
		"-Wno-cast-align",
	}
	defines {
		"PTR64=1",
	}
end

if (_OPTIONS["PLATFORM"]=="riscv64") then
	defines {
		"PTR64=1",
	}
end

if (_OPTIONS["PLATFORM"]=="mips64") then
	defines {
		"PTR64=1",
	}
end

local subdir
if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
	subdir = _OPTIONS["osd"] .. "/" .. _OPTIONS["target"]
else
	subdir = _OPTIONS["osd"] .. "/" .. _OPTIONS["target"] .. _OPTIONS["subtarget"]
end

if not toolchain(MAME_BUILD_DIR, subdir) then
	return -- no action specified
end

configuration { "asmjs" }
	buildoptions {
		"-std=gnu89",
		"-Wno-implicit-function-declaration",
		"-s USE_SDL_TTF=2",
	}
	buildoptions_cpp {
		"-std=c++17",
		"-s DISABLE_EXCEPTION_CATCHING=2",
		"-s EXCEPTION_CATCHING_WHITELIST=\"['_ZN15running_machine17start_all_devicesEv','_ZN12cli_frontend7executeEiPPc','_ZN8chd_file11open_commonEb','_ZN8chd_file13read_metadataEjjRNSt3__212basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE','_ZN8chd_file13read_metadataEjjRNSt3__26vectorIhNS0_9allocatorIhEEEE','_ZNK19netlist_mame_device19base_validity_checkER16validity_checker']\"",
	}
	linkoptions {
		"-Wl,--start-group",
		"-r",
	}
	archivesplit_size "20"

configuration { "android*" }
	buildoptions {
		"-Wno-undef",
		"-Wno-typedef-redefinition",
		"-Wno-unknown-warning-option",
		"-Wno-incompatible-ms-struct",
	}
	buildoptions_cpp {
		"-std=c++17",
		"-Wno-extern-c-compat",
		"-Wno-tautological-constant-out-of-range-compare",
		"-Wno-tautological-pointer-compare",
	}
	archivesplit_size "20"

configuration { "android-arm64" }
	buildoptions {
		"-Wno-asm-operand-widths",
	}

configuration { "pnacl" }
	buildoptions {
		"-std=gnu89",
		"-Wno-inline-new-delete",
	}
	buildoptions_cpp {
		"-std=c++17",
	}
	archivesplit_size "20"

configuration { "linux-* or rpi or ci20"}
		links {
			"dl",
			"rt",
		}
		if _OPTIONS["distro"]=="debian-stable" then
			defines
			{
				"NO_AFFINITY_NP",
			}
		end



configuration { "steamlink" }
	links {
		"dl",
		"EGL",
		"GLESv2",
		"SDL2",
	}
	defines {
		"EGL_API_FB",
	}

configuration { "rpi" }
	links {
		"SDL2",
		"fontconfig",
		"X11",
		"GLESv2",
		"EGL",
		"bcm_host",
		"vcos",
		"vchiq_arm",
		"pthread",
	}


configuration { "ci20" }
	links {
		"SDL2",
		"asound",
		"fontconfig",
		"freetype",
		"pthread",
	}


configuration { "osx* or xcode4" }
		links {
			"pthread",
		}

configuration { "mingw*" }
		if _OPTIONS["osd"]=="sdl" then
			linkoptions {
				"-Wl,--start-group",
			}
		else
			linkoptions {
				"-static",
			}
			flags {
				"LinkSupportCircularDependencies",
			}
		end
		links {
			"user32",
			"winmm",
			"advapi32",
			"shlwapi",
			"wsock32",
			"ws2_32",
			"psapi",
			"iphlpapi",
			"shell32",
			"userenv",
		}

configuration { "vsllvm" }
	defines {
		"XML_STATIC",
		"WIN32",
		"_WIN32",
		"_CRT_NONSTDC_NO_DEPRECATE",
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_STDIO_LEGACY_WIDE_SPECIFIERS",
	}
	includedirs {
		MAME_DIR .. "3rdparty/dxsdk/Include"
	}

configuration { "vs20*" }
		defines {
			"XML_STATIC",
			"WIN32",
			"_WIN32",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"_CRT_SECURE_NO_DEPRECATE",
			"_CRT_STDIO_LEGACY_WIDE_SPECIFIERS",
		}

-- Windows Store/Phone projects already link against the available libraries.
if _OPTIONS["vs"]==nil or not (string.startswith(_OPTIONS["vs"], "winstore8") or string.startswith(_OPTIONS["vs"], "winphone8")) then
		links {
			"user32",
			"winmm",
			"advapi32",
			"shlwapi",
			"wsock32",
			"ws2_32",
			"psapi",
			"iphlpapi",
			"shell32",
			"userenv",
		}
end

		buildoptions {
			"/WX",     -- Treats all compiler warnings as errors.
			"/w45038", -- warning C5038: data member 'member1' will be initialized after data member 'member2'
		}

		buildoptions {
			"/wd4003", -- warning C4003: not enough actual parameters for macro 'xxx'
			"/wd4005", -- warning C4005: The macro identifier is defined twice. The compiler uses the second macro definition
			"/wd4018", -- warning C4018: 'x' : signed/unsigned mismatch
			"/wd4060", -- warning C4060: switch statement contains no 'case' or 'default' labels
			"/wd4065", -- warning C4065: switch statement contains 'default' but no 'case' labels
			"/wd4100", -- warning C4100: 'xxx' : unreferenced formal parameter
			"/wd4127", -- warning C4127: conditional expression is constant
			"/wd4146", -- warning C4146: unary minus operator applied to unsigned type, result still unsigned
			"/wd4201", -- warning C4201: nonstandard extension used : nameless struct/union
			"/wd4244", -- warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
			"/wd4245", -- warning C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
			"/wd4250", -- warning C4250: 'xxx' : inherits 'xxx' via dominance
			"/wd4267", -- warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
			"/wd4310", -- warning C4310: cast truncates constant value
			"/wd4319", -- warning C4319: 'operator' : zero extending 'type' to 'type' of greater size
			"/wd4324", -- warning C4324: 'xxx' : structure was padded due to __declspec(align())
			"/wd4334", -- warning C4334: '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
			"/wd4389", -- warning C4389: 'operator' : signed/unsigned mismatch
			"/wd4456", -- warning C4456: declaration of 'xxx' hides previous local declaration
			"/wd4457", -- warning C4457: declaration of 'xxx' hides function parameter
			"/wd4458", -- warning C4458: declaration of 'xxx' hides class member
			"/wd4459", -- warning C4459: declaration of 'xxx' hides global declaration
			"/wd4702", -- warning C4702: unreachable code
			"/wd4706", -- warning C4706: assignment within conditional expression
			"/wd4804", -- warning C4804: '>>': unsafe use of type 'bool' in operation
			"/wd4805", -- warning C4805: 'x' : unsafe mix of type 'xxx' and type 'xxx' in operation
			"/wd4996", -- warning C4996: 'function': was declared deprecated
		}

if _OPTIONS["vs"]=="intel-15" then
		buildoptions {
			"/Qwd9",                -- remark #9: nested comment is not allowed
			"/Qwd82",               -- remark #82: storage class is not first
			"/Qwd111",              -- remark #111: statement is unreachable
			"/Qwd128",              -- remark #128: loop is not reachable
			"/Qwd177",              -- remark #177: function "xxx" was declared but never referenced
			"/Qwd181",              -- remark #181: argument of type "UINT32={unsigned int}" is incompatible with format "%d", expecting argument of type "int"
			"/Qwd185",              -- remark #185: dynamic initialization in unreachable code
			"/Qwd280",              -- remark #280: selector expression is constant
			"/Qwd344",              -- remark #344: typedef name has already been declared (with same type)
			"/Qwd411",              -- remark #411: class "xxx" defines no constructor to initialize the following
			"/Qwd869",              -- remark #869: parameter "xxx" was never referenced
			"/Qwd2545",             -- remark #2545: empty dependent statement in "else" clause of if - statement
			"/Qwd2553",             -- remark #2553: nonstandard second parameter "TCHAR={WCHAR = { __wchar_t } } **" of "main", expected "char *[]" or "char **" extern "C" int _tmain(int argc, TCHAR **argv)
			"/Qwd2557",             -- remark #2557: comparison between signed and unsigned operands
			"/Qwd3280",             -- remark #3280: declaration hides member "attotime::seconds" (declared at line 126) static attotime from_seconds(INT32 seconds) { return attotime(seconds, 0); }

			"/Qwd170",              -- error #170: pointer points outside of underlying object
			"/Qwd188",              -- error #188: enumerated type mixed with another type

			"/Qwd63",               -- warning #63: shift count is too large
			"/Qwd177",              -- warning #177: label "xxx" was declared but never referenced
			"/Qwd186",              -- warning #186: pointless comparison of unsigned integer with zero
			"/Qwd488",              -- warning #488: template parameter "_FunctionClass" is not used in declaring the parameter types of function template "device_delegate<_Signature>::device_delegate<_FunctionClass>(delegate<_Signature>:
			"/Qwd1478",             -- warning #1478: function "xxx" (declared at line yyy of "zzz") was declared deprecated
			"/Qwd1879",             -- warning #1879: unimplemented pragma ignored
			"/Qwd3291",             -- warning #3291: invalid narrowing conversion from "double" to "int"
			"/Qwd1195",             -- error #1195: conversion from integer to smaller pointer
			"/Qwd47",               -- error #47: incompatible redefinition of macro "xxx"
			"/Qwd265",              -- error #265: floating-point operation result is out of range
			-- these occur on a release build, while we can increase the size limits instead some of the files do require extreme amounts
			"/Qwd11074",            -- remark #11074: Inlining inhibited by limit max-size  / remark #11074: Inlining inhibited by limit max-total-size
			"/Qwd11075",            -- remark #11075: To get full report use -Qopt-report:4 -Qopt-report-phase ipo
		}
end

if _OPTIONS["vs"]=="clangcl" then
		buildoptions {
			"-Wno-enum-conversion",
			"-Wno-ignored-qualifiers",
			"-Wno-missing-braces",
			"-Wno-missing-field-initializers",
			"-Wno-new-returns-null",
			"-Wno-nonportable-include-path",
			"-Wno-pointer-bool-conversion",
			"-Wno-pragma-pack",
			"-Wno-switch",
			"-Wno-tautological-constant-out-of-range-compare",
			"-Wno-tautological-pointer-compare",
			"-Wno-unknown-warning-option",
			"-Wno-unused-const-variable",
			"-Wno-unused-function",
			"-Wno-unused-label",
			"-Wno-unused-local-typedef",
			"-Wno-unused-private-field",
			"-Wno-unused-variable",
		}
end

		linkoptions {
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
		}
		includedirs {
			MAME_DIR .. "3rdparty/dxsdk/Include"
		}
configuration { "winphone8* or winstore8*" }
	linkoptions {
		"/ignore:4264" -- LNK4264: archiving object file compiled with /ZW into a static library; note that when authoring Windows Runtime types it is not recommended to link with a static library that contains Windows Runtime metadata
	}
configuration { "vsllvm" }
		buildoptions {
			"-Wno-tautological-constant-out-of-range-compare",
			"-Wno-ignored-qualifiers",
			"-Wno-missing-field-initializers",
			"-Wno-ignored-pragma-optimize",
			"-Wno-unknown-warning-option",
			"-Wno-unused-function",
			"-Wno-unused-label",
			"-Wno-unused-local-typedef",
			"-Wno-unused-const-variable",
			"-Wno-unused-parameter",
			"-Wno-unneeded-internal-declaration",
			"-Wno-unused-private-field",
			"-Wno-missing-braces",
			"-Wno-unused-variable",
			"-Wno-tautological-pointer-compare",
			"-Wno-nonportable-include-path",
			"-Wno-enum-conversion",
			"-Wno-pragma-pack",
			"-Wno-new-returns-null",
			"-Wno-sign-compare",
			"-Wno-switch",
			"-Wno-tautological-undefined-compare",
			"-Wno-deprecated-declarations",
			"-Wno-macro-redefined",
			"-Wno-narrowing",
		}


configuration { }

if (_OPTIONS["SOURCES"] ~= nil) then
	local str = _OPTIONS["SOURCES"]
	local sourceargs = ""
	for word in string.gmatch(str, '([^,]+)') do
		if (not os.isfile(path.join(MAME_DIR, word))) then
			print("File " .. word.. " does not exist")
			os.exit()
		end
		sourceargs = sourceargs .. " " .. word
	end
	OUT_STR = os.outputof( PYTHON .. " " .. MAME_DIR .. "scripts/build/makedep.py sourcesproject -r " .. MAME_DIR .. " -t " .. _OPTIONS["subtarget"] .. sourceargs )
	load(OUT_STR)()
	os.outputof( PYTHON .. " " .. MAME_DIR .. "scripts/build/makedep.py sourcesfilter" .. sourceargs .. " > ".. GEN_DIR  .. _OPTIONS["target"] .. "/" .. _OPTIONS["subtarget"] .. ".flt" )
end

group "libs"

if (not os.isfile(path.join("src", "osd",  _OPTIONS["osd"] .. ".lua"))) then
	error("Unsupported value '" .. _OPTIONS["osd"] .. "' for OSD")
end
dofile(path.join("src", "osd", _OPTIONS["osd"] .. ".lua"))
dofile(path.join("src", "lib.lua"))
if opt_tool(MACHINES, "NETLIST") then
   dofile(path.join("src", "netlist.lua"))
end
--if (STANDALONE~=true) then
dofile(path.join("src", "formats.lua"))
formatsProject(_OPTIONS["target"],_OPTIONS["subtarget"])
--end

group "3rdparty"
dofile(path.join("src", "3rdparty.lua"))


group "core"

dofile(path.join("src", "emu.lua"))

if (STANDALONE~=true) then
	dofile(path.join("src", "mame", "frontend.lua"))
end

group "devices"
dofile(path.join("src", "devices.lua"))
devicesProject(_OPTIONS["target"],_OPTIONS["subtarget"])

if (STANDALONE~=true) then
	group "drivers"
	findfunction("createProjects_" .. _OPTIONS["target"] .. "_" .. _OPTIONS["subtarget"])(_OPTIONS["target"], _OPTIONS["subtarget"])
end

group "emulator"
dofile(path.join("src", "main.lua"))
if (_OPTIONS["SOURCES"] == nil) then
	if (_OPTIONS["target"] == _OPTIONS["subtarget"]) then
		startproject (_OPTIONS["target"])
	else
		if (_OPTIONS["subtarget"]=="mess") then
			startproject (_OPTIONS["subtarget"])
		else
			startproject (_OPTIONS["target"] .. _OPTIONS["subtarget"])
		end
	end
else
	startproject (_OPTIONS["subtarget"])
end
mainProject(_OPTIONS["target"],_OPTIONS["subtarget"])
strip()

if _OPTIONS["with-tools"] then
	group "tools"
	dofile(path.join("src", "tools.lua"))
end

if _OPTIONS["with-tests"] then
	group "tests"
	dofile(path.join("src", "tests.lua"))
end

if _OPTIONS["with-benchmarks"] then
	group "benchmarks"
	dofile(path.join("src", "benchmarks.lua"))
end

function generate_has_header(hashname, hash)
   fname = GEN_DIR .. "has_" .. hashname:lower() .. ".h"
   file = io.open(fname, "w")
   file:write("// Generated file, edition is futile\n")
   file:write("\n")
   file:write(string.format("#ifndef GENERATED_HAS_%s_H\n", hashname))
   file:write(string.format("#define GENERATED_HAS_%s_H\n", hashname))
   file:write("\n")
   for k, v in pairs(hash) do
	  if v then
		 file:write(string.format("#define HAS_%s_%s\n", hashname, k))
	  end
   end
   file:write("\n")
   file:write("#endif\n")
   file:close()
end

generate_has_header("CPUS", CPUS)
generate_has_header("SOUNDS", SOUNDS)
generate_has_header("MACHINES", MACHINES)
generate_has_header("VIDEOS", VIDEOS)
generate_has_header("BUSES", BUSES)
generate_has_header("FORMATS", FORMATS)

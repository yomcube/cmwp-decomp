#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "20100817_DBG", # 0
    "20100817_REL", # 1
    "20090820_REL", # 2
]
DEFAULT_VERSION_STRING = VERSIONS[DEFAULT_VERSION]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=DEFAULT_VERSION_STRING,
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.7.5"
config.objdiff_tag = "v3.6.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.json"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
cflags_base_all = [
    "-nodefaults",
    "-proc gekko",
    "-fp hardware",
    "-enum int",
    "-cpp_exceptions off",
    "-nosyspath",
    "-cwd include",
    "-i include",
    "-i include/stdlib",
    "-i include/RVLFaceLib",
    "-i include/Runtime.PPCEABI.H",
    "-enc SJIS",
    "-DTARGET_RVL",
    "-DSLAM_" + config.version[:-4],
    "-DSLAM_VERSION=" + config.version[:-4],
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"'
]

# Warning flags
if args.warn == "all":
    cflags_base_all.append("-W all")
elif args.warn == "off":
    cflags_base_all.append("-W off")
elif args.warn == "error":
    cflags_base_all.append("-W error")

# Build flags
cflags_base_debug = [
    *cflags_base_all,
    "-opt off",
    "-inline off",
    "-g",
    "-DDEBUG",
]

cflags_base_release = [
    *cflags_base_all,
    "-O4,p",
    "-inline auto",
    "-ipa file",
    "-DNDEBUG",
]

if config.version.endswith("DBG"):
    cflags_base = cflags_base_debug
else:
    cflags_base = cflags_base_release

cflags_base_exceptions = [
    *cflags_base,
    "-cpp_exceptions on",
]

# Slam flags
if config.version.startswith("2010"):
    cflags_slam = [
        *cflags_base_debug,
        "-cpp_exceptions on"
    ]
else:
    cflags_slam = [
        *cflags_base,
        "-func_align 4",
        "-cpp_exceptions on"
    ]

if config.version.endswith("REL"):
    cflags_slam.append("-use_lmw_stmw on")

# Metroworks libraries flags
cflags_runtime = [
    *cflags_base_all,
    "-O4,p",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-func_align 4",
    "-gccinc"
]

cflags_trk = [
    *cflags_runtime,
    "-inline deferred",
    "-sdata 0"
]

cflags_msl = [
    *cflags_base_all,
    "-O4,p",
    "-func_align 4",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-D_IEEE_LIBM",
    "-use_lmw_stmw on"
]

config.linker_version = "Wii/1.0"

if config.version.startswith("2010"):
    sdk_compiler = "Wii/1.0"
else:
    sdk_compiler = "GC/3.0a5.2"

Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


MatchingDbg = MatchingFor("20100817_DBG")
MatchingRel = MatchingFor(["20100817_REL", "20090820_REL"])
Matching2009 = MatchingFor("20090820_REL")
Matching2010 = MatchingFor(["20100817_REL", "20100817_DBG"])


def SLAM(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "slamWii",
        "mw_version": config.linker_version,
        "cflags": cflags_slam,
        "progress_category": "slam",
        "objects": objects,
    }


def RVLSDKLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": sdk_compiler,
        "cflags": cflags_base,
        "progress_category": "main_libs",
        "objects": objects,
    }


def RFL(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "RVLFaceLib",
        "mw_version": "GC/3.0a5.2",
        "cflags": cflags_base_exceptions,
        # "progress_category": "main_libs",
        "objects": objects,
    }


def Runtime(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
         "progress_category": "main_libs",
        "objects": objects,
    }


def TRK(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "TRK_Hollywood_Revolution",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
         "progress_category": "main_libs",
        "objects": objects,
    }


def MSL_C(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "MSL_C.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_msl,
        "progress_category": "main_libs",
        "objects": objects,
    }


def MSL_CPP(objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": "MSL_C++.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_msl,
        "progress_category": "main_libs",
        "objects": objects,
    }


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    SLAM([
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MyHomeButton.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/AudioWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/Stream.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/Subband.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/TPLHelper.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/LightPng.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/LightZ.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/iconLoader.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/AutoSave_Wii.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/adler32.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/compress.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/crc32.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/deflate.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/inffast.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/inflate.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/inftrees.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/trees.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/uncompr.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/zutil.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/FileSystemWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/GraphicsWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/GlobalVars.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/LoadingScreen.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/main.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MainInit.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MemoryWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/Mouse.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/PadInput.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/PlatformWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/RenderTarget_Wii.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/Reset.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/TextureWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/TimerWII.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/WooFile.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/ZeroBuffer.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/SlamAssertWii.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MenuMisc.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/Multimedia.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MenuFileHandling.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/SoundChannel.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/FlicPlayer.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/FMV.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/MobiClipWii.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/THPAudioDecode.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/THPDraw.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/THPPlayer.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/THPRead.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Wii_RSO/THPVideoDecode.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/bitwise.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/block.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/codebook.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/floor0.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/floor1.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/framing.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/info.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/ivorbisfile_example.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/mapping0.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/mdct.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/registry.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/res012.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/sharedbook.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/synthesis.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/vorbisfile.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tremor/window.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PGFimage.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/Encoder.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/Decoder.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/WaveletTransform.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/AutoSave.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SlamPAK.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/Platform.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformAudio.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformAudioDictionary.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformTexture.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformTextureDictionary.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformTEXTDictionary.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/PlatformTextureHelper.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/unicode.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/AnimationResource.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/BaseSlamRuntime.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/BitPacker.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/CachedVertexManager.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/crc.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/DllServices.cpp"),
        Object(Matching,    "SLAM_TECHNOLOGY/Slam_Core/lz.c"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuAnimation.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuAnimationControl.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuBackdrop.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuContainer.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuDebugWatchConsole.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuDefineList.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuDefs.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuDialog.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuEffect.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuFlicPlayer.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuFont.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/menuinterpreter.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/menuInterpreterProcess.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuInterpreterStructs.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuItem.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuObject.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuPage.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuScriptHandler.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuScrollingText.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuSubPage.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuTimeSlicedFunctions.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuTextFunctions.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuTimer.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/MenuVariable.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/mt19937ar.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SlamAssert.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SlamDebugData.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMMaths.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMPluginInterface.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMRandom.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMSprites.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SlamString.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/StateSystemManager.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/StringFunctions.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/RenderTarget.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/RuntimeExpression.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMPluginManager.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tinystr.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tinyxml.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tinyxmlerror.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/tinyxmlparser.cpp"),
        Object(NonMatching, "SLAM_TECHNOLOGY/Slam_Core/SLAMParticles.cpp")
    ]),

    Runtime([
        Object(Matching,    "Runtime.PPCEABI.H/__mem.c"),
        Object(Matching,    "Runtime.PPCEABI.H/__va_arg.c"),
        Object(Matching,    "Runtime.PPCEABI.H/global_destructor_chain.c"),
        Object(NonMatching, "Runtime.PPCEABI.H/CPlusLibPPC.cp"),
        Object(NonMatching, "Runtime.PPCEABI.H/New.cp"),
        Object(NonMatching, "Runtime.PPCEABI.H/NewMore.cp"),
        Object(NonMatching, "Runtime.PPCEABI.H/NMWException.cp"),
        Object(Matching,    "Runtime.PPCEABI.H/ptmf.c"),
        Object(NonMatching, "Runtime.PPCEABI.H/MWRTTI.cp"),
        Object(Matching,    "Runtime.PPCEABI.H/runtime.c"),
        Object(Matching,    "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        Object(Matching,    "Runtime.PPCEABI.H/Gecko_setjmp.c"),
        Object(NonMatching, "Runtime.PPCEABI.H/Gecko_ExceptionPPC.cp"),
        Object(NonMatching, "Runtime.PPCEABI.H/GCN_mem_alloc.c"),
        Object(Matching,    "Runtime.PPCEABI.H/ppcsfpe.c", extra_cflags=["-sdata 0"])
    ]),
    
    MSL_C([
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/alloc.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/ansi_files.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Src/ansi_fp.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/arith.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/assert.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/bsearch.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/buffer_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/char_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/ctype.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/direct_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/errno.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/file_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/FILE_POS.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/locale.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/mbstring.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/mem.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/mem_funcs.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/math_api.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/misc_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/printf.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/qso.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/fenv.ppc.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/rand.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/scanf.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/signal.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/string.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/strtold.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/strtoul.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/time.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/float.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wcstold.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wcstoul.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wctrans.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wctype.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wmem.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wprintf.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wscanf.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wstring.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/wchar_io.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/sysenv.GCN.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/assert.gcn.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/uart_console_io_gcn.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/abort_exit_ppc_eabi.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/secure_error.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/math_double.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/math_float.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/math_longdouble.c"),

        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_acos.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_acosh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_asin.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_atan2.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_atanh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_cosh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_exp.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_fmod.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_gamma.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_gamma_r.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_hypot.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_lgamma.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_lgamma_r.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_log.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_log10.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_pow.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_remainder.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_sinh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/fminmaxdim.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/k_cos.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/k_sin.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/k_standard.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/k_tan.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_asinh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_atan.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_cbrt.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_ceil.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_copysign.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_cos.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_erf.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_expm1.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_floor.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_frexp.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_ilogb.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_lib_version.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_log1p.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_logb.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_matherr.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_modf.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_nextafter.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_rint.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_signgam.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_sin.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_tan.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/s_tanh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_acos.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_acosh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_asin.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_atan2.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_atanh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_cosh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_exp.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_fmod.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_gamma.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_hypot.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_lgamma.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_log.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_log10.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_pow.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_remainder.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_sinh.c"),
        Object(Matching,    "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Math/Double_precision/w_sqrt.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common_Embedded/Src/math_sun.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/PPC_EABI/SRC/math_ppc.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/extras.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/stat.c"),
        Object(NonMatching, "MSL_C.PPCEABI.bare.H/MSL_Common/Src/stdio_posix.c")
    ]),
    
    MSL_CPP([
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/ios.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/iostream.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/locale.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/MSLstring.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/msl_mutex.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/strstream.cpp"),
        Object(NonMatching, "MSL_C++.PPCEABI.bare.H/MSL_Common/Src/msl_thread.cpp")
    ]),
    
    TRK([
        Object(NonMatching, "TRK_Hollywood_Revolution/__exception.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/targsupp.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/cc_ddh.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/cc_gdev.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/MWEvent_gc.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/MWTrace.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/MWCriticalSection_gc.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/custconn/CircleBuffer.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/flush_cache.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/main_TRK.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/mainloop.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/mem_TRK.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/dispatch.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/dolphin_trk.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/dolphin_trk_glue.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/notify.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/nubevent.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/nubinit.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/serpoll.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/string_TRK.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/support.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/mpc_7xx_603e.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/msg.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/msgbuf.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/msghndlr.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/mslsupp.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/targimpl.c"),
        Object(NonMatching, "TRK_Hollywood_Revolution/target_options.c")
    ]),

    RVLSDKLib("homebuttonLib", [
        Object(NonMatching, "homebuttonLib/HBMFrameController.cpp"),
        Object(NonMatching, "homebuttonLib/HBMAnmController.cpp"),
        Object(NonMatching, "homebuttonLib/HBMGUIManager.cpp"),
        Object(NonMatching, "homebuttonLib/HBMController.cpp"),
        Object(NonMatching, "homebuttonLib/HBMRemoteSpk.cpp"),
        Object(NonMatching, "homebuttonLib/HBMAxSound.cpp"),
        Object(NonMatching, "homebuttonLib/HBMCommon.cpp"),
        Object(NonMatching, "homebuttonLib/HBMBase.cpp"),

        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_animation.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_arcResourceAccessor.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_bounding.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_common.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_drawInfo.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_group.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_init.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_layout.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_material.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_pane.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_picture.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_resourceAccessor.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_textBox.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/lyt_window.cpp"),

        Object(NonMatching, "homebuttonLib/nw4hbm/math_arithmetic.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/math_equation.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/math_geometry.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/math_triangular.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/math_types.cpp"),

        Object(NonMatching, "homebuttonLib/nw4hbm/ut_binaryFileFormat.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_CharStrmReader.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_CharWriter.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_Font.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_LinkList.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_list.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_ResFont.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_ResFontBase.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_RomFont.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_TagProcessorBase.cpp"),
        Object(NonMatching, "homebuttonLib/nw4hbm/ut_TextWriterBase.cpp"),

        Object(NonMatching, "homebuttonLib/sound/mix.cpp"),
        Object(NonMatching, "homebuttonLib/sound/syn.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synctrl.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synenv.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synmix.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synpitch.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synsample.cpp"),
        Object(NonMatching, "homebuttonLib/sound/synvoice.cpp"),
        Object(NonMatching, "homebuttonLib/sound/seq.cpp")
    ]),


    RVLSDKLib("tpl", [
        Object(NonMatching, "RVL_SDK/tpl/TPL.c")
    ]),

    RVLSDKLib("thp", [
        Object(NonMatching, "RVL_SDK/thp/THPDec.c"),
        Object(NonMatching, "RVL_SDK/thp/THPStats.c"),
        Object(NonMatching, "RVL_SDK/thp/THPAudio.c")
    ]),

    RVLSDKLib("rso", [
        Object(NonMatching, "RVL_SDK/rso/RSOLink.c"),
    ]),

    RVLSDKLib("NdevExi2A", [
        Object(NonMatching, "RVL_SDK/NdevExi2A/DebuggerDriver.c"),
        Object(NonMatching, "RVL_SDK/NdevExi2A/exi2.c")
    ]),

    RVLSDKLib("amcstubs", [
        Object(Matching,    "RVL_SDK/amcstubs/AmcExi2Stubs.c")
    ]),

    RVLSDKLib("odenotstub", [
        Object(Matching,    "RVL_SDK/odenotstub/odenotstub.c")
    ]),

    RVLSDKLib("ai", [
        Object(NonMatching, "RVL_SDK/ai/ai.c")
    ]),

    RVLSDKLib("arc", [
        Object(Matching,    "RVL_SDK/arc/arc.c")
    ]),

    RVLSDKLib("axart", [
        Object(NonMatching, "RVL_SDK/axart/axart.c"),
        Object(NonMatching, "RVL_SDK/axart/axartsound.c"),
        Object(NonMatching, "RVL_SDK/axart/axartcents.c"),
        Object(NonMatching, "RVL_SDK/axart/axartenv.c"),
        Object(NonMatching, "RVL_SDK/axart/axartlfo.c"),
        Object(NonMatching, "RVL_SDK/axart/axart3d.c"),
        Object(NonMatching, "RVL_SDK/axart/axartlpf.c")
    ]),

    RVLSDKLib("ax", [
        Object(NonMatching, "RVL_SDK/ax/AX.c"),
        Object(NonMatching, "RVL_SDK/ax/AXAlloc.c"),
        Object(NonMatching, "RVL_SDK/ax/AXAux.c"),
        Object(NonMatching, "RVL_SDK/ax/AXCL.c"),
        Object(NonMatching, "RVL_SDK/ax/AXOut.c"),
        Object(NonMatching, "RVL_SDK/ax/AXSPB.c"),
        Object(NonMatching, "RVL_SDK/ax/AXVPB.c"),
        Object(NonMatching, "RVL_SDK/ax/AXProf.c"),
        Object(NonMatching, "RVL_SDK/ax/AXComp.c"),
        Object(NonMatching, "RVL_SDK/ax/DSPCode.c"),
        Object(NonMatching, "RVL_SDK/ax/DSPADPCM.c")
    ]),

    RVLSDKLib("axfx", [
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbHi.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbHiDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbHiExp.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbHiExpDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXDelay.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXDelayDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXDelayExp.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXDelayExpDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbStd.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbStdDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbStdExp.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXReverbStdExpDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXChorus.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXChorusDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXChorusExp.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXChorusExpDpl2.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXLfoTable.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXSrcCoef.c"),
        Object(NonMatching, "RVL_SDK/axfx/AXFXHooks.c")
    ]),

    RVLSDKLib("base", [
        Object(Matching,    "RVL_SDK/base/PPCArch.c"),
        Object(Matching,    "RVL_SDK/base/PPCPm.c")
    ]),

    RVLSDKLib("bte", [
        Object(NonMatching, "RVL_SDK/bte/rvl/gki_buffer.c"),
        Object(NonMatching, "RVL_SDK/bte/rvl/gki_debug.c"),
        Object(NonMatching, "RVL_SDK/bte/rvl/gki_time.c"),
        Object(NonMatching, "RVL_SDK/bte/rvl/gki_ppc.c"),
        Object(NonMatching, "RVL_SDK/bte/rvl/hcisu_h2.c"),
        Object(NonMatching, "RVL_SDK/bte/rvl/uusb_ppc.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/aa/bta_aa_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/ag/bta_ag_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/av/bta_av_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/bi/bta_bi_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/cg/bta_cg_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/ct/bta_ct_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dg/bta_dg_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dm/bta_dm_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/fs/bta_fs_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/ft/bta_ft_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hd/bta_hd_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hh/bta_hh_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/pbc/bta_pbc_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/pbs/bta_pbs_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/pr/bta_pr_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/ss/bta_ss_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/bta_sys_cfg.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_disp.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_hcisu.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_init.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_load.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_logmsg.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_main.c"),
        Object(NonMatching, "RVL_SDK/bte/main/bte_version.c"),
        Object(NonMatching, "RVL_SDK/bte/btu/btu_task1.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/bd.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/bta_sys_conn.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/bta_sys_main.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/ptim.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/sys/utl.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/prm/bta_prm_api.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/prm/bta_prm_main.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/fs/bta_fs_ci.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dm/bta_dm_act.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dm/bta_dm_api.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dm/bta_dm_main.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/dm/bta_dm_pm.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hd/bta_hd_act.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hd/bta_hd_api.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hd/bta_hd_main.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hh/bta_hh_act.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hh/bta_hh_api.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hh/bta_hh_main.c"),
        Object(NonMatching, "RVL_SDK/bte/bta/hh/bta_hh_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_acl.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_dev.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_devctl.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_discovery.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_inq.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_main.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_pm.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_sco.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btm/btm_sec.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btu/btu_hcif.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/btu/btu_init.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/wbt/wbt_ext.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/gap/gap_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/gap/gap_conn.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/gap/gap_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/goep/goep_trace.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/goep/goep_util.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/goep/goep_fs.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hcic/hcicmds.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidd_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidd_conn.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidd_mgmt.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidd_pm.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidh_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/hid/hidh_conn.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/l2cap/l2c_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/l2cap/l2c_csm.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/l2cap/l2c_link.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/l2cap/l2c_main.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/l2cap/l2c_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/port_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/port_rfc.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/port_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_l2cap_if.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_mx_fsm.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_port_fsm.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_port_if.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_ts_frames.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/rfcomm/rfc_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_api.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_db.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_discovery.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_main.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_server.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/sdp/sdp_utils.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/xml/xml_bld.c"),
        Object(NonMatching, "RVL_SDK/bte/stack/xml/xml_parse.c")
    ]),

    RVLSDKLib("cx", [
        Object(NonMatching, "RVL_SDK/cx/CXCompression.c"),
        Object(NonMatching, "RVL_SDK/cx/CXStreamingUncompression.c"),
        Object(NonMatching, "RVL_SDK/cx/CXUncompression.c"),
        Object(NonMatching, "RVL_SDK/cx/CXSecureUncompression.c")
    ]),

    RVLSDKLib("darch", [
        Object(NonMatching, "RVL_SDK/darch/darch.c")
    ]),

    RVLSDKLib("db", [
        Object(Matching,    "RVL_SDK/db/db.c")
    ]),

    RVLSDKLib("dsp", [
        Object(NonMatching, "RVL_SDK/dsp/dsp.c"),
        Object(NonMatching, "RVL_SDK/dsp/dsp_debug.c"),
        Object(NonMatching, "RVL_SDK/dsp/dsp_perf.c"),
        Object(NonMatching, "RVL_SDK/dsp/dsp_task.c")
    ]),

    RVLSDKLib("dvd", [
        Object(NonMatching, "RVL_SDK/dvd/dvdfs.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvd.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvdqueue.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvderror.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvdidutils.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvdFatal.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvdDeviceError.c"),
        Object(NonMatching, "RVL_SDK/dvd/dvd_broadway.c")
    ]),

    RVLSDKLib("enc", [
        Object(NonMatching, "RVL_SDK/enc/encutility.c"),
        Object(NonMatching, "RVL_SDK/enc/encunicode.c"),
        Object(NonMatching, "RVL_SDK/enc/encjapanese.c"),
        Object(NonMatching, "RVL_SDK/enc/enclatin.c"),
        Object(NonMatching, "RVL_SDK/enc/encconvert.c"),
        Object(NonMatching, "RVL_SDK/enc/encchinese.c"),
        Object(NonMatching, "RVL_SDK/enc/enckorean.c")
    ]),

    RVLSDKLib("esp", [
        Object(NonMatching, "RVL_SDK/esp/esp.c")
    ]),

    RVLSDKLib("euart", [
        Object(NonMatching, "RVL_SDK/euart/euart.c")
    ]),

    RVLSDKLib("exi", [
        Object(NonMatching, "RVL_SDK/exi/EXIBios.c"),
        Object(NonMatching, "RVL_SDK/exi/EXIUart.c"),
        Object(NonMatching, "RVL_SDK/exi/EXIAd16.c"),
        Object(NonMatching, "RVL_SDK/exi/EXICommon.c")
    ]),

    RVLSDKLib("fs", [
        Object(NonMatching, "RVL_SDK/fs/fs.c")
    ]),

    RVLSDKLib("gx", [
        Object(NonMatching, "RVL_SDK/gx/GXInit.c"),
        Object(NonMatching, "RVL_SDK/gx/GXFifo.c"),
        Object(NonMatching, "RVL_SDK/gx/GXAttr.c"),
        Object(NonMatching, "RVL_SDK/gx/GXMisc.c"),
        Object(NonMatching, "RVL_SDK/gx/GXGeometry.c"),
        Object(NonMatching, "RVL_SDK/gx/GXFrameBuf.c"),
        Object(NonMatching, "RVL_SDK/gx/GXLight.c"),
        Object(NonMatching, "RVL_SDK/gx/GXTexture.c"),
        Object(NonMatching, "RVL_SDK/gx/GXBump.c"),
        Object(NonMatching, "RVL_SDK/gx/GXTev.c"),
        Object(NonMatching, "RVL_SDK/gx/GXPixel.c"),
        Object(NonMatching, "RVL_SDK/gx/GXDraw.c"),
        Object(NonMatching, "RVL_SDK/gx/GXDisplayList.c"),
        Object(NonMatching, "RVL_SDK/gx/GXVert.c"),
        Object(NonMatching, "RVL_SDK/gx/GXTransform.c"),
        Object(NonMatching, "RVL_SDK/gx/GXVerify.c"),
        Object(NonMatching, "RVL_SDK/gx/GXVerifXF.c"),
        Object(NonMatching, "RVL_SDK/gx/GXVerifRAS.c"),
        Object(NonMatching, "RVL_SDK/gx/GXSave.c"),
        Object(NonMatching, "RVL_SDK/gx/GXPerf.c")
    ]),

    RVLSDKLib("hid", [
        Object(NonMatching, "RVL_SDK/hid/hid_api.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_ios.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_client.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_device.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_interface.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_open_close.c"),
        Object(NonMatching, "RVL_SDK/hid/hid_task.c")
    ]),

    RVLSDKLib("hio2", [
        Object(MatchingDbg, "RVL_SDK/hio2/hio2.c", mw_version="GC/3.0a5.2")
    ]),

    RVLSDKLib("ipc", [
        Object(NonMatching, "RVL_SDK/ipc/ipcMain.c"),
        Object(NonMatching, "RVL_SDK/ipc/ipcclt.c"),
        Object(NonMatching, "RVL_SDK/ipc/memory.c"),
        Object(NonMatching, "RVL_SDK/ipc/ipcProfile.c")
    ]),

    RVLSDKLib("kbd", [
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib.c"),
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib_led.c"),
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib_init.c"),
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib_maps_us.c"),
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib_maps_jp.c"),
        Object(NonMatching, "RVL_SDK/kbd/kbd_lib_maps_eu.c")
    ]),

    RVLSDKLib("kpad", [
        Object(NonMatching, "RVL_SDK/kpad/KPAD.c"),
        Object(NonMatching, "RVL_SDK/kpad/KMPLS.c"),
        Object(NonMatching, "RVL_SDK/kpad/KZMplsTestSub.c")
    ]),

    RVLSDKLib("kpr", [
        Object(NonMatching, "RVL_SDK/kpr/kpr_lib.c")
    ]),

    RVLSDKLib("mem", [
        Object(NonMatching, "RVL_SDK/mem/mem_heapCommon.c"),
        Object(NonMatching, "RVL_SDK/mem/mem_expHeap.c"),
        Object(NonMatching, "RVL_SDK/mem/mem_frameHeap.c"),
        Object(NonMatching, "RVL_SDK/mem/mem_unitHeap.c"),
        Object(NonMatching, "RVL_SDK/mem/mem_allocator.c"),
        Object(NonMatching, "RVL_SDK/mem/mem_list.c")
    ]),

    RVLSDKLib("midi", [
        Object(Matching,    "RVL_SDK/midi/MIDI.c"),
        Object(Matching,    "RVL_SDK/midi/MIDIRead.c"),
        Object(Matching,    "RVL_SDK/midi/MIDIXfer.c")
    ]),

    RVLSDKLib("mix", [
        Object(NonMatching, "RVL_SDK/mix/mix.c"),
        Object(NonMatching, "RVL_SDK/mix/remote.c")
    ]),

    RVLSDKLib("mtx", [
        Object(NonMatching, "RVL_SDK/mtx/mtx.c"),
        Object(NonMatching, "RVL_SDK/mtx/mtxvec.c"),
        Object(NonMatching, "RVL_SDK/mtx/mtxstack.c"),
        Object(NonMatching, "RVL_SDK/mtx/mtx44.c"),
        Object(NonMatching, "RVL_SDK/mtx/mtx44vec.c"),
        Object(NonMatching, "RVL_SDK/mtx/vec.c"),
        Object(NonMatching, "RVL_SDK/mtx/quat.c"),
        Object(NonMatching, "RVL_SDK/mtx/psmtx.c")
    ]),

    RVLSDKLib("nand", [
        Object(NonMatching, "RVL_SDK/nand/nand.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDOpenClose.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDCore.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDSecret.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDCheck.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDLogging.c"),
        Object(NonMatching, "RVL_SDK/nand/NANDErrorMessage.c")
    ]),

    RVLSDKLib("os", [
        Object(NonMatching, "RVL_SDK/os/OS.c"),
        Object(NonMatching, "RVL_SDK/os/OSAddress.c"),
        Object(NonMatching, "RVL_SDK/os/OSAlarm.c"),
        Object(NonMatching, "RVL_SDK/os/OSAlloc.c"),
        Object(NonMatching, "RVL_SDK/os/OSArena.c"),
        Object(NonMatching, "RVL_SDK/os/OSAudioSystem.c"),
        Object(NonMatching, "RVL_SDK/os/OSCache.c"),
        Object(NonMatching, "RVL_SDK/os/OSContext.c"),
        Object(NonMatching, "RVL_SDK/os/OSError.c"),
        Object(NonMatching, "RVL_SDK/os/OSExec.c"),
        Object(NonMatching, "RVL_SDK/os/OSFatal.c"),
        Object(NonMatching, "RVL_SDK/os/OSFont.c"),
        Object(NonMatching, "RVL_SDK/os/OSInterrupt.c"),
        Object(NonMatching, "RVL_SDK/os/OSLink.c"),
        Object(NonMatching, "RVL_SDK/os/OSMessage.c"),
        Object(NonMatching, "RVL_SDK/os/OSMemory.c"),
        Object(NonMatching, "RVL_SDK/os/OSMutex.c"),
        Object(NonMatching, "RVL_SDK/os/OSReboot.c"),
        Object(NonMatching, "RVL_SDK/os/OSReset.c"),
        Object(NonMatching, "RVL_SDK/os/OSRtc.c"),
        Object(NonMatching, "RVL_SDK/os/OSSemaphore.c"),
        Object(NonMatching, "RVL_SDK/os/OSStopwatch.c"),
        Object(NonMatching, "RVL_SDK/os/OSSync.c"),
        Object(NonMatching, "RVL_SDK/os/OSThread.c"),
        Object(NonMatching, "RVL_SDK/os/OSTime.c"),
        Object(NonMatching, "RVL_SDK/os/OSTimer.c"),
        Object(NonMatching, "RVL_SDK/os/OSUtf.c"),
        Object(NonMatching, "RVL_SDK/os/OSIpc.c"),
        Object(NonMatching, "RVL_SDK/os/OSStateTM.c"),
        Object(NonMatching, "RVL_SDK/os/time.dolphin.c"),
        Object(NonMatching, "RVL_SDK/os/init/__start.c"),
        Object(NonMatching, "RVL_SDK/os/OSPlayRecord.c"),
        Object(NonMatching, "RVL_SDK/os/OSStateFlags.c"),
        Object(NonMatching, "RVL_SDK/os/OSNet.c"),
        Object(NonMatching, "RVL_SDK/os/OSNandbootInfo.c"),
        Object(NonMatching, "RVL_SDK/os/OSPlayTime.c"),
        Object(NonMatching, "RVL_SDK/os/OSInstall.c"),
        Object(NonMatching, "RVL_SDK/os/OSCrc.c"),
        Object(NonMatching, "RVL_SDK/os/OSLaunch.c"),
        Object(NonMatching, "RVL_SDK/os/init/__ppc_eabi_init.cpp")
    ]),

    RVLSDKLib("pad", [
        Object(NonMatching, "RVL_SDK/pad/Padclamp.c"),
        Object(NonMatching, "RVL_SDK/pad/Pad.c")
    ]),

    RVLSDKLib("sc", [
        Object(NonMatching, "RVL_SDK/sc/scsystem.c"),
        Object(NonMatching, "RVL_SDK/sc/scapi.c"),
        Object(NonMatching, "RVL_SDK/sc/scapi_net.c"),
        Object(NonMatching, "RVL_SDK/sc/scapi_prdinfo.c")
    ]),

    RVLSDKLib("seq", [
        Object(NonMatching, "RVL_SDK/seq/seq.c")
    ]),

    RVLSDKLib("si", [
        Object(NonMatching, "RVL_SDK/si/SIBios.c"),
        Object(NonMatching, "RVL_SDK/si/SISamplingRate.c"),
        Object(NonMatching, "RVL_SDK/si/SISteering.c"),
        Object(NonMatching, "RVL_SDK/si/SISteeringXfer.c"),
        Object(NonMatching, "RVL_SDK/si/SISteeringAuto.c")
    ]),

    RVLSDKLib("sp", [
        Object(NonMatching, "RVL_SDK/sp/sp.c")
    ]),

    RVLSDKLib("syn", [
        Object(NonMatching, "RVL_SDK/syn/syn.c"),
        Object(NonMatching, "RVL_SDK/syn/synctrl.c"),
        Object(NonMatching, "RVL_SDK/syn/synenv.c"),
        Object(NonMatching, "RVL_SDK/syn/synlfo.c"),
        Object(NonMatching, "RVL_SDK/syn/synmix.c"),
        Object(NonMatching, "RVL_SDK/syn/synpitch.c"),
        Object(NonMatching, "RVL_SDK/syn/synsample.c"),
        Object(NonMatching, "RVL_SDK/syn/synvoice.c"),
        Object(NonMatching, "RVL_SDK/syn/synwt.c")
    ]),

    RVLSDKLib("usb", [
        Object(NonMatching, "RVL_SDK/usb/usb.c")
    ]),

    RVLSDKLib("usb", [
        Object(NonMatching, "RVL_SDK/vi/vi.c"),
        Object(NonMatching, "RVL_SDK/vi/i2c.c"),
        Object(NonMatching, "RVL_SDK/vi/vi3in1.c")
    ]),

    RVLSDKLib("wbc", [
        Object(NonMatching, "RVL_SDK/wbc/wbc.c", mw_version="GC/3.0a5.2")
    ]),

    RVLSDKLib("wenc", [
        Object(Matching,    "RVL_SDK/wenc/wenc.c", mw_version = "GC/3.0a5.2")
    ]),

    RVLSDKLib("wpad", [
        Object(NonMatching, "RVL_SDK/wpad/WPAD.c"),
        Object(NonMatching, "RVL_SDK/wpad/WPADHIDParser.c"),
        Object(NonMatching, "RVL_SDK/wpad/WPADEncrypt.c"),
        Object(NonMatching, "RVL_SDK/wpad/WPADClamp.c"),
        Object(NonMatching, "RVL_SDK/wpad/WPADMem.c"),
        Object(NonMatching, "RVL_SDK/wpad/lint.c"),
        Object(NonMatching, "RVL_SDK/wpad/WUD.c"),
        Object(NonMatching, "RVL_SDK/wpad/WUDHidHost.c"),
        Object(NonMatching, "RVL_SDK/wpad/debug_msg.c")
    ]),

    RVLSDKLib("wud", [
        Object(NonMatching, "RVL_SDK/wud/WUD.c"),
        Object(NonMatching, "RVL_SDK/wud/WUDHidHost.c"),
        Object(NonMatching, "RVL_SDK/wud/debug_msg.c")
    ]),

    RFL([
        Object(Matching,    "RVLFaceLib/RFL_System.c"),
        Object(Matching,    "RVLFaceLib/RFL_NANDLoader.c"),
        Object(Matching,    "RVLFaceLib/RFL_NANDAccess.c"),
        Object(MatchingRel, "RVLFaceLib/RFL_Model.c"),
        Object(MatchingRel, "RVLFaceLib/RFL_MakeTex.c"),
        Object(Matching,    "RVLFaceLib/RFL_Icon.c"),
        Object(Matching,    "RVLFaceLib/RFL_HiddenDatabase.c"),
        Object(Matching,    "RVLFaceLib/RFL_Database.c"),
        Object(Matching,    "RVLFaceLib/RFL_Controller.c"),
        Object(Matching,    "RVLFaceLib/RFL_MiddleDatabase.c"),
        Object(Matching,    "RVLFaceLib/RFL_MakeRandomFace.c"),
        Object(Matching,    "RVLFaceLib/RFL_DefaultDatabase.c"),
        Object(Matching,    "RVLFaceLib/RFL_DataUtility.c"),
        Object(Matching,    "RVLFaceLib/RFL_NWC24.c"),
        Object(Matching,    "RVLFaceLib/RFL_Format.c")
    ])
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("slam", "SLAM Technology Runtime"),
    ProgressCategory("main_libs", "Libraries"),
]
config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)

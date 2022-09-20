#pragma once

using namespace Falcor;

#if DEBUG
#define SCENES_FOLDER "../Source/RTRad/Scenes"
#define RENDERPASSES_FOLDER "RTRad/RTRad/RenderPasses"
#else
#define SCENES_FOLDER "RTRad/Scenes"
#define RENDERPASSES_FOLDER "RTRad/RTRad/RenderPasses"
#endif

#define DEFAULT_SCENE SCENES_FOLDER"/CornellLucy.pyscene"

#define CITPASS_H <RTRad/RenderPasses/CITPass/CITPass.h>
#define CITPASS_DIR_SHADERS RENDERPASSES_FOLDER"/CITPass/Shaders"

#define VITPASS_H <RTRad/RenderPasses/VITPass/VITPass.h>
#define VITPASS_DIR_SHADERS RENDERPASSES_FOLDER"/VITPass/Shaders"
#define VITPASS_DIR_UVPLANESCENE SCENES_FOLDER"/UVPlaneScene/UVPlane.pyscene"

#define CVMPASS_H <RTRad/RenderPasses/CVMPass/CVMPass.h>
#define CVMPASS_DIR_SHADERS RENDERPASSES_FOLDER"/CVMPass/Shaders"

#define RTLPASS_H <RTRad/RenderPasses/RTLPass/RTLightmapPass.h>
#define RTLPASS_DIR_SHADERS RENDERPASSES_FOLDER"/RTLPass/Shaders"

#define SETTINGSOBJ_H <RTRad/Core/SettingsObject.h>

#define DEFAULT_WIN_HEIGHT 810
#define DEFAULT_WIN_WIDTH 1440
#define ASPECT_RATIO (float)DEFAULT_WIN_WIDTH / (float)DEFAULT_WIN_HEIGHT

#define CLEAR_COLOR float4(0.38f, 0.52f, 0.10f, 1)

#define DEFAULT_MIPMAP_LEVELS 4

#define MAX_HEMISPHERIC_SAMPLES 1024

#define MAX_VISCACHE_RESOLUTION 512

#define MAX_SAMPLES_PER_BATCH 268435456

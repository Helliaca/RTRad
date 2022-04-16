#pragma once

using namespace Falcor;

#define SHADERS_FOLDER "RTRad/RTRad/RenderPasses/Shaders"

#define CITPASS_H <RTRad/RenderPasses/CITPass/CITPass.h>
#define CITPASS_DIR_SHADERS "RTRad/RTRad/RenderPasses/CITPass/Shaders"

#define VITPASS_H <RTRad/RenderPasses/VITPass/VITPass.h>
#define VITPASS_DIR_SHADERS "RTRad/RTRad/RenderPasses/VITPass/Shaders"
#define VITPASS_DIR_UVPLANESCENE "../Source/RTRad/RTRad/RenderPasses/VITPass/UVPlaneScene/UVPlane.pyscene"

#define CVMPASS_H <RTRad/RenderPasses/CVMPass/CVMPass.h>
#define CVMPASS_DIR_SHADERS "RTRad/RTRad/RenderPasses/CVMPass/Shaders"

#define RTLPASS_H <RTRad/RenderPasses/RTLPass/RTLightmapPass.h>
#define RTLPASS_DIR_SHADERS "RTRad/RTRad/RenderPasses/RTLPass/Shaders"

#define SETTINGSOBJ_H <RTRad/Core/SettingsObject.h>

#define SCENES_FOLDER "../Source/RTRad/Scenes"

#define DEFAULT_SCENE SCENES_FOLDER"/CornellLucy.pyscene"

#define DEFAULT_WIN_HEIGHT 1080
#define DEFAULT_WIN_WIDTH 1920

#define CLEAR_COLOR float4(0.38f, 0.52f, 0.10f, 1)

#define DEFAULT_MIPMAP_LEVELS 4

#define HEMISPHERIC_SAMPLING 0

#define MAX_VISCACHE_RESOLUTION 512

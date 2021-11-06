#pragma once

#include "Falcor.h"

using namespace Falcor;

#define SHADERS_FOLDER "RTRad/RenderPasses/Shaders"

#define SCENES_FOLDER "../Source/RTRad/Scenes"

#define DEFAULT_SCENE SCENES_FOLDER"/CornellTest.pyscene"

#define DEFAULT_WIN_HEIGHT 1080
#define DEFAULT_WIN_WIDTH 1920

#define CLEAR_COLOR float4(0.38f, 0.52f, 0.10f, 1)

struct TextureGroup {
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr matTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;
};

#pragma once

#include "Falcor.h"

using namespace Falcor;

#define SHADERS_FOLDER "RTRad/RenderPasses/Shaders"

#define SCENES_FOLDER "../Source/RTRad/Scenes"

#define DEFAULT_SCENE SCENES_FOLDER"/CornellLucy.pyscene"

struct TextureGroup {
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr matTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;
};

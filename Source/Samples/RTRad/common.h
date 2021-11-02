#pragma once

#include "Falcor.h"

using namespace Falcor;

#define SHADERS_FOLDER "Samples/RTRad/RenderPasses/Shaders"

struct TextureGroup {
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr matTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;
};

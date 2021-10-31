#pragma once

#include "Falcor.h"

using namespace Falcor;

struct TextureGroup {
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;
};

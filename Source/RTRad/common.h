#pragma once

#include "Falcor.h"

using namespace Falcor;

#define SHADERS_FOLDER "RTRad/RenderPasses/Shaders"

#define SCENES_FOLDER "../Source/RTRad/Scenes"

#define DEFAULT_SCENE SCENES_FOLDER"/CornellLucy.pyscene"

#define DEFAULT_WIN_HEIGHT 1080
#define DEFAULT_WIN_WIDTH 1920

#define CLEAR_COLOR float4(0.38f, 0.52f, 0.10f, 1)

#define DEFAULT_MIPMAP_LEVELS 4

#define HEMISPHERIC_SAMPLING 0

#define MAX_VISCACHE_RESOLUTION 512

struct TextureGroup {
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr matTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;

    Buffer::SharedPtr visBuf;

    void generateLMips(RenderContext* pRenderContext) {
        lgiTex->generateMips(pRenderContext);
        lgoTex->generateMips(pRenderContext);
    }

    static TextureGroup makeTextures(int res, bool useVisCache)
    {
        TextureGroup ret;

        if (useVisCache) {
            /*
            * A word on the math down here to calculate the size of the viscache:
            * res^4 is the raw amount of pairs there are in a set of res*res.
            * We then multiply by 4, because each uint has 4 bytes, and we need to give the amount of bytes
            * Then we divide by 2, because since we only want each UNIQUE pair, we only need half
            * Then we divide by 32, because a uint has 32bits, and we store visibilty in individual bits
            * Then we subtract res*res, because we don't need mirrored pairs like (0,0), (1,1) etc. There are exactly res*res of these
            */

            // We have to use size_t here because otherwise we overflow the integer
            size_t rres = std::min(res, MAX_VISCACHE_RESOLUTION);
            size_t bufSize = (rres * rres * rres * rres * (size_t)4 / (size_t)2 / (size_t)32);
            bufSize -= (rres * rres);

            ret.visBuf = Buffer::create(bufSize,
                Falcor::ResourceBindFlags::ShaderResource | Falcor::ResourceBindFlags::UnorderedAccess,
                Falcor::Buffer::CpuAccess::None
            );
        }
        else
        {
            ret.visBuf = NULL;
        }

        ret.posTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        ret.nrmTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        ret.arfTex = Texture::create2D(res, res, Falcor::ResourceFormat::R32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        ret.matTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        ret.lgiTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        ret.lgoTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);

        return ret;
    }
};

#pragma once


#include "Falcor.h"
#include "BasePipelineElement.h"
#include <RTRad/Core/common.h>
#include SETTINGSOBJ_H

using namespace Falcor;

struct TextureGroupSettings : public RR::BaseSettings {
    bool useMipmaps;
    bool useVoxelmap;
    bool useViscache;
    bool useConservativeRasterization;

    uint32_t voxelResolution;
    uint32_t textureResolution;

    TextureGroupSettings() {
        useMipmaps = true;
        useVoxelmap = true;
        useViscache = false;
        useConservativeRasterization = false;

        voxelResolution = 64;
        textureResolution = 64;
    }
};

class TextureGroup : public RR::SettingsStruct<TextureGroupSettings>, public RR::BasePipelineElement {
public:
    Texture::SharedPtr posTex;
    Texture::SharedPtr nrmTex;
    Texture::SharedPtr arfTex;
    Texture::SharedPtr matTex;
    Texture::SharedPtr lgiTex;
    Texture::SharedPtr lgoTex;

    Texture::SharedPtr voxTex;

    Buffer::SharedPtr visBuf;

    Fbo::SharedPtr outputFbo;

    bool settingsChanged;

    TextureGroup() {
        settings = TextureGroupSettings();
    }

    void onRenderGui(Gui* Gui, Gui::Window* win) override {
        win->text("TextureGroup Settings");

        Falcor::Gui::DropdownList reslst;
        reslst.push_back({ 32, "32" });
        reslst.push_back({ 64, "64" });
        reslst.push_back({ 128, "128" });
        reslst.push_back({ 256, "256" });
        reslst.push_back({ 384, "384" });
        reslst.push_back({ 512, "512" });
        reslst.push_back({ 768, "768" });
        reslst.push_back({ 1024, "1024" });
        reslst.push_back({ 1536, "1536" });
        reslst.push_back({ 2048, "2048" });

        uint32_t prevRes = settings.textureResolution;
        win->dropdown("Lightmap Resolution", reslst, settings.textureResolution);

        Falcor::Gui::DropdownList vreslst;
        vreslst.push_back({ 32, "32" });
        vreslst.push_back({ 64, "64" });
        vreslst.push_back({ 128, "128" });
        vreslst.push_back({ 256, "256" });

        uint32_t vprevRes = settings.voxelResolution;
        win->dropdown("VoxelMap Resolution", vreslst, settings.voxelResolution);

        bool prevVisc = settings.useViscache;
        if (settings.textureResolution <= MAX_VISCACHE_RESOLUTION) {
            win->checkbox("Use Viscache", settings.useViscache);
        }

        bool prevCons = settings.useConservativeRasterization;
        win->checkbox("Cons. Rasterization", settings.useConservativeRasterization);

        settingsChanged =
            prevRes != settings.textureResolution ||
            vprevRes != settings.voxelResolution ||
            prevVisc != settings.useViscache ||
            prevCons != settings.useConservativeRasterization ||
            settings.useViscache && !visBuf
            ;
    }

    void generateLMips(RenderContext* pRenderContext) {
        lgiTex->generateMips(pRenderContext);
        lgoTex->generateMips(pRenderContext);
    }

    void RemakeTextures(Fbo::SharedPtr outputFbo)
    {
        settingsChanged = false;
        this->outputFbo = outputFbo;

        if (settings.textureResolution > MAX_VISCACHE_RESOLUTION) settings.useViscache = false;

        if (settings.useViscache) {
            /*
            * A word on the math down here to calculate the size of the viscache:
            * res^4 is the raw amount of pairs there are in a set of res*res.
            * We then multiply by 4, because each uint has 4 bytes, and we need to give the amount of bytes
            * Then we divide by 2, because since we only want each UNIQUE pair, we only need half
            * Then we divide by 32, because a uint has 32bits, and we store visibilty in individual bits
            * Then we subtract res*res, because we don't need mirrored pairs like (0,0), (1,1) etc. There are exactly res*res of these
            */

            // We have to use size_t here because otherwise we overflow the integer
            size_t rres = std::min((int)settings.textureResolution, MAX_VISCACHE_RESOLUTION);
            size_t bufSize = (rres * rres * rres * rres * (size_t)4 / (size_t)2 / (size_t)32);
            bufSize -= (rres * rres);

            visBuf = Buffer::create(bufSize,
                Falcor::ResourceBindFlags::ShaderResource | Falcor::ResourceBindFlags::UnorderedAccess,
                Falcor::Buffer::CpuAccess::None
            );
        }
        else
        {
            visBuf = nullptr;
        }

        int voxres = settings.voxelResolution, res = settings.textureResolution;

        if (settings.useVoxelmap) {
            voxTex = Texture::create3D(voxres, voxres, voxres, ResourceFormat::RGBA16Float, 1, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        }

        posTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        nrmTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        arfTex = Texture::create2D(res, res, Falcor::ResourceFormat::R32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        matTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        lgiTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
        lgoTex = Texture::create2D(res, res, Falcor::ResourceFormat::RGBA32Float, 1U, DEFAULT_MIPMAP_LEVELS, (const void*)nullptr, Falcor::ResourceBindFlags::UnorderedAccess | Falcor::ResourceBindFlags::RenderTarget | Falcor::ResourceBindFlags::ShaderResource);
    }
};

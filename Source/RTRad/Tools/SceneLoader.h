#pragma once

#include "Falcor.h"

using namespace Falcor;

class SceneLoader {
public:
    inline static bool LoadSceneFromFile(const std::string& filename, Scene::SharedPtr& scene, Camera::SharedPtr& camera) {
        scene = Scene::create(filename);
        if (!scene) return false;

        camera = scene->getCamera();

        // Update the controllers
        float radius = scene->getSceneBounds().radius();
        scene->setCameraSpeed(radius * 0.25f);
        float nearZ = std::max(0.1f, radius / 750.0f);
        float farZ = radius * 10;
        camera->setDepthRange(nearZ, farZ);
        camera->setAspectRatio((float)gpFramework->getTargetFbo().get()->getWidth() / (float)gpFramework->getTargetFbo().get()->getWidth());

        return true;
    }
};

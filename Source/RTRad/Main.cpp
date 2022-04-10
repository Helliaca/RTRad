#include <RTRad/RTRad.h>


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    RTRad::UniquePtr pRenderer = std::make_unique<RTRad>();

    SampleConfig config;
    config.windowDesc.title = "RTRad Project";
    config.windowDesc.resizableWindow = true;
    config.windowDesc.mode = Falcor::Window::WindowMode::Normal;
    config.windowDesc.height = DEFAULT_WIN_HEIGHT;
    config.windowDesc.width = DEFAULT_WIN_WIDTH;

    Sample::run(config, pRenderer);

    return 0;
}

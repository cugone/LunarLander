#include "Engine/Core/EngineBase.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Platform/Win.hpp"

#include "Game/Game.hpp"

#include <string>

#pragma warning(push)
#pragma warning(disable: 28251)

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    using namespace std::literals::string_literals;
    UNUSED(hInstance);
    UNUSED(hPrevInstance);
    UNUSED(nCmdShow);
    UNUSED(pCmdLine);
    Engine<Game>::Initialize("Lunar Lander"s);
    Engine<Game>::Run();
    Engine<Game>::Shutdown();
}

#pragma warning(pop)

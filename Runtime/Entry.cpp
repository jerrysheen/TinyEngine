#include "PreCompiledHeader.h"
#include "Core/Game.h"

std::unique_ptr<EngineCore::Game> EngineCore::Game::m_Instance = nullptr;
int main(int argc, char* argv[])
{
    EngineCore::Game::GetInstance().Launch();
    std::cout << "APP Runing!!!!" << std::endl;
    return 0;

}
#include "PreCompiledHeader.h"
#include "Core/Game.h"

std::unique_ptr<EngineCore::Game> EngineCore::Game::m_Instance = nullptr;
int main(int argc, char* argv[])
{
    std::cout << "APP Runing!!!!" << std::endl;
    EngineCore::Game::GetInstance()->Launch();
    std::cout << "APP ShutDown!!!!" << std::endl;
    return 0;

}
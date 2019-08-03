#include "GameConsole.h"

GameConsole::GameConsole() {
    // ctor
}

GameConsole::~GameConsole() {
    // dtor
}

void GameConsole::Print(std::string str) { outFunction(str); }

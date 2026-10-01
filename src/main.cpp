/************************************************************************
 *                               Forradia                               *
 *                                                                      *
 * Copyright (c) 2026 Andreas Åkerberg                                  *
 * SPDX-License-Identifier: MIT                                         *
 ************************************************************************/

#include "Game.hpp"

int main(int argc, char *argv[])
{
    using namespace Forradia;

    Game::Instance().Start();

    return 0;
}
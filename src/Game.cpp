/************************************************************************
 *                               Forradia                               *
 *                                                                      *
 * Copyright (c) 2026 Andreas Åkerberg                                  *
 * SPDX-License-Identifier: MIT                                         *
 ************************************************************************/

#include "Game.hpp"

namespace Forradia
{
    void Game::Engine::Common::Matter::Geometry::PointF::operator+=(const PointF &other)
    {
        x += other.x;
        y += other.y;
    }

    Game::Engine::Common::Matter::Geometry::PointF
    Game::Engine::Common::Matter::Geometry::PointF::operator+(const PointF &other) const
    {
        return {x + other.x, y + other.y};
    }

    Game::Engine::Common::Matter::Geometry::PointF
    Game::Engine::Common::Matter::Geometry::PointF::operator-(const PointF &other) const
    {
        return {x - other.x, y - other.y};
    }

    SDL_Color Game::Engine::Common::Matter::Coloring::Color::ToSDLColor()
    {
        return {static_cast<Uint8>(r * 255), static_cast<Uint8>(g * 255),
                static_cast<Uint8>(b * 255), static_cast<Uint8>(a * 255)};
    }

    void Game::Start()
    {
        std::cout << "Game started" << std::endl;

        engine_.Run();
    }

    void Game::Engine::Run()
    {
        std::cout << "Engine running" << std::endl;

        srand(time(nullptr));

        imageBank_.LoadImages();

        rendering_.textRenderer_.Initialize();

        while (running_)
        {
            PollEvents();

            minorComponents_.cursor_.Reset();

            sceneManager_.UpdateCurrentScene();

            minorComponents_.fpsCounter_.Update();

            sdlDevice_.ClearCanvas();

            sceneManager_.RenderCurrentScene();

            minorComponents_.fpsCounter_.Render();

            minorComponents_.cursor_.Render();

            sdlDevice_.PresentCanvas();
        }
    }

    void Game::Engine::Common::SDLDeleter::operator()(SDL_Window *window)
    {
        SDL_DestroyWindow(window);
    }

    void Game::Engine::Common::SDLDeleter::operator()(SDL_Renderer *renderer)
    {
        SDL_DestroyRenderer(renderer);
    }

    void Game::Engine::Common::SDLDeleter::operator()(SDL_Surface *surface)
    {
        SDL_FreeSurface(surface);
    }

    void Game::Engine::Common::SDLDeleter::operator()(SDL_Texture *texture)
    {
        SDL_DestroyTexture(texture);
    }

    void Game::Engine::Common::SDLDeleter::operator()(TTF_Font *font)
    {
        TTF_CloseFont(font);
    }

    std::string Game::Engine::Common::StringUtilities::Replace(std::string_view text,
                                                               std::string_view oldValue,
                                                               std::string_view newValue)
    {
        std::string result(text);

        size_t position{0};

        while ((position = result.find(oldValue, position)) != std::string::npos)
        {
            result.replace(position, oldValue.length(), newValue);
            position += newValue.length();
        }

        return result;
    }

    std::string Game::Engine::Common::FilePathUtilities::GetFileNameNoExt(std::string_view path)
    {
        auto fileName{std::filesystem::path(path).filename().string()};

        return fileName.substr(0, fileName.find_last_of("."));
    }

    Game::Engine::Common::Matter::Geometry::Size
    Game::Engine::Common::CanvasUtilities::GetCanvasSize()
    {
        auto &sldDevice{Game::Instance().engine_.sdlDevice_};

        int width{0};
        int height{0};

        SDL_GetWindowSize(sldDevice.window_.get(), &width, &height);

        return {width, height};
    }

    float Game::Engine::Common::CanvasUtilities::GetAspectRatio()
    {
        auto canvasSize{GetCanvasSize()};

        return static_cast<float>(canvasSize.width) / canvasSize.height;
    }

    float Game::Engine::Common::CanvasUtilities::ConvertWidthToHeight(float width)
    {
        return width * GetAspectRatio();
    }

    float Game::Engine::Common::CanvasUtilities::ConvertHeightToWidth(float height)
    {
        return height / GetAspectRatio();
    }

    int Game::Engine::Common::TimeUtilities::Now()
    {
        return SDL_GetTicks();
    }

    Game::Engine::Common::Matter::Geometry::PointF
    Game::Engine::Common::MouseUtilities::GetMousePosition()
    {
        auto &GetCanvasSize = Game::Engine::Common::CanvasUtilities::GetCanvasSize;

        auto &sldDevice{Game::Instance().engine_.sdlDevice_};

        auto canvasSize{GetCanvasSize()};

        int x;
        int y;

        SDL_GetMouseState(&x, &y);

        return {static_cast<float>(x) / canvasSize.width,
                static_cast<float>(y) / canvasSize.height};
    }

    void Game::Engine::PollEvents()
    {
        auto &Now = Game::Engine::Common::TimeUtilities::Now;

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
            {
                running_ = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                sceneManager_.OnKeyDownCurrentScene(event.key.keysym.sym);
                break;
            }
            case SDL_KEYUP:
            {
                sceneManager_.OnKeyUpCurrentScene(event.key.keysym.sym);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                sceneManager_.OnMouseDownCurrentScene(event.button.button);

                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    ticksLeftMouseButtonDown_ = Now();
                    break;
                case SDL_BUTTON_RIGHT:
                    ticksRightMouseButtonDown_ = Now();
                    break;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                auto clickSpeed{0};

                switch (event.button.button)
                {
                case SDL_BUTTON_LEFT:
                    clickSpeed = Now() - ticksLeftMouseButtonDown_;
                    break;
                case SDL_BUTTON_RIGHT:
                    clickSpeed = Now() - ticksRightMouseButtonDown_;
                    break;
                }
                sceneManager_.OnMouseUpCurrentScene(event.button.button, clickSpeed);
                break;
            }
            }
        }
    }

    Game::Engine::SDLDevice::SDLDevice()
    {
        using SDLDeleter = Game::Engine::Common::SDLDeleter;

        auto windowFlags{SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP};

        window_ = std::shared_ptr<SDL_Window>(
            SDL_CreateWindow(k_windowName_.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             800, 600, windowFlags),
            SDLDeleter());

        renderer_ = std::shared_ptr<SDL_Renderer>(
            SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED), SDLDeleter());
    }

    void Game::Engine::SDLDevice::ClearCanvas()
    {
        SDL_SetRenderDrawColor(renderer_.get(), 0, 150, 255, 255);
        SDL_RenderClear(renderer_.get());
    }

    void Game::Engine::SDLDevice::PresentCanvas()
    {
        SDL_RenderPresent(renderer_.get());
    }

    void Game::Engine::ImageBank::LoadImages()
    {
        auto &Replace = Game::Engine::Common::StringUtilities::Replace;

        auto imagesDirectory{SDL_GetBasePath() + std::string(k_relativeImagesDirectory_)};

        imagesDirectory = Replace(imagesDirectory, "\\", "/");

        auto rdi{std::filesystem::recursive_directory_iterator(imagesDirectory)};

        for (const auto &entry : rdi)
        {
            if (entry.is_regular_file() && entry.path().extension() == ".png")
            {
                LoadSingleImage(entry.path().string());
            }
        }
    }

    void Game::Engine::ImageBank::LoadSingleImage(std::string_view fullPath)
    {
        auto &Hash = Game::Engine::Common::Hash;
        auto &Replace = Game::Engine::Common::StringUtilities::Replace;
        auto &GetFileNameNoExt = Game::Engine::Common::FilePathUtilities::GetFileNameNoExt;

        using SDLDeleter = Game::Engine::Common::SDLDeleter;

        auto &sldDevice{Game::Instance().engine_.sdlDevice_};

        std::string path{Replace(fullPath, "\\", "/")};

        auto pureName{GetFileNameNoExt(path)};

        auto hash{Hash(pureName)};

        auto loaded{IMG_Load(path.c_str())};

        if (!loaded)
        {
            std::cout << "Failed to load image " << path << ": " << IMG_GetError() << std::endl;
            return;
        }

        auto converted{SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0)};

        SDL_FreeSurface(loaded);

        if (!converted)
        {
            std::cout << "Failed to convert image " << path << ": " << SDL_GetError() << std::endl;
            return;
        }

        auto surface{std::shared_ptr<SDL_Surface>(converted, SDLDeleter())};

        auto texture{std::shared_ptr<SDL_Texture>(
            SDL_CreateTextureFromSurface(sldDevice.renderer_.get(), surface.get()), SDLDeleter())};

        ImageEntry entry{texture, surface};

        images_.insert({hash, entry});
    }

    std::shared_ptr<SDL_Texture> Game::Engine::ImageBank::GetImage(int imageNameHash)
    {
        if (images_.contains(imageNameHash))
        {
            return images_.at(imageNameHash).texture;
        }

        return nullptr;
    }

    Game::Engine::Common::Matter::Geometry::Size
    Game::Engine::ImageBank::GetImageSize(int imageNameHash)
    {
        auto width{0};
        auto height{0};

        if (images_.contains(imageNameHash))
        {
            SDL_QueryTexture(images_.at(imageNameHash).texture.get(), nullptr, nullptr, &width,
                             &height);
        }

        return {width, height};
    }

    bool Game::Engine::ImageBank::IsPixelVisible(int imageNameHash, float x, float y)
    {
        if (images_.contains(imageNameHash))
        {
            auto surface{images_.at(imageNameHash).surface.get()};

            if (!surface || !surface->pixels || surface->w <= 0 || surface->h <= 0 || x < 0.0f ||
                y < 0.0f || x >= 1.0f || y >= 1.0f)
            {
                return false;
            }

            auto xPx{static_cast<int>(x * static_cast<float>(surface->w))};
            auto yPx{static_cast<int>(y * static_cast<float>(surface->h))};

            if (xPx < 0)
            {
                xPx = 0;
            }

            if (yPx < 0)
            {
                yPx = 0;
            }

            if (xPx >= surface->w)
            {
                xPx = surface->w - 1;
            }

            if (yPx >= surface->h)
            {
                yPx = surface->h - 1;
            }

            auto pixels{static_cast<Uint8 *>(surface->pixels)};
            auto alpha{pixels[yPx * surface->pitch + xPx * 4 + 3]};

            return alpha > 0;
        }

        return false;
    }

    void Game::Engine::MinorComponents::FPSCounter::Update()
    {
        auto &Now = Game::Engine::Common::TimeUtilities::Now;

        auto now{Now()};

        if (now - ticksLastUpdate_ > Game::Engine::Common::Constants::k_oneSecondMillis)
        {
            fps_ = framesCount_;

            framesCount_ = 0;

            ticksLastUpdate_ = now;
        }

        framesCount_++;
    }

    void Game::Engine::MinorComponents::FPSCounter::Render()
    {
        auto &textRenderer{Game::Instance().engine_.rendering_.textRenderer_};

        std::string fpsText{std::to_string(fps_) + " FPS"};

        textRenderer.DrawString(fpsText, 0.93f, 0.05f);
    }

    Game::Engine::MinorComponents::Cursor::Cursor()
    {
        SDL_ShowCursor(SDL_DISABLE);
    }

    void Game::Engine::MinorComponents::Cursor::Reset()
    {
        cursorStyle_ = CursorStyles::Default;
    }

    void Game::Engine::MinorComponents::Cursor::Render()
    {
        auto &imageRenderer = Game::Instance().engine_.rendering_.imageRenderer_;

        auto &ConvertWidthToHeight = Game::Engine::Common::CanvasUtilities::ConvertWidthToHeight;

        auto &GetMousePosition = Game::Engine::Common::MouseUtilities::GetMousePosition;

        auto mousePosition{GetMousePosition()};

        auto cursorWidth{k_cursorSize_};
        auto cursorHeight{ConvertWidthToHeight(cursorWidth)};

        std::string cursorImage;

        switch (cursorStyle_)
        {
        case CursorStyles::Hovering:
            cursorImage = "CursorHovering";
            break;
        case CursorStyles::Default:
        default:
            cursorImage = "CursorDefault";
            break;
        }

        imageRenderer.DrawImage(cursorImage, mousePosition.x - cursorWidth / 2,
                                mousePosition.y - cursorHeight / 2, cursorWidth, cursorHeight);
    }

    Game::Engine::SceneManager::SceneManager()
    {
        AddScene("IntroScene", std::make_shared<IntroScene>());
        AddScene("MainMenuScene", std::make_shared<MainMenuScene>());

        GoToScene("IntroScene");
    }

    void Game::Engine::SceneManager::AddScene(std::string_view sceneName,
                                              std::shared_ptr<IScene> scene)
    {
        auto &Hash = Game::Engine::Common::Hash;

        scenes_.insert({Hash(sceneName), scene});
    }

    void Game::Engine::SceneManager::GoToScene(std::string_view sceneName)
    {
        auto &Hash = Game::Engine::Common::Hash;

        currentScene_ = Hash(sceneName);
    }

    void Game::Engine::SceneManager::UpdateCurrentScene()
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_[currentScene_]->Update();
        }
    }

    void Game::Engine::SceneManager::RenderCurrentScene()
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_[currentScene_]->Render();
        }
    }

    void Game::Engine::SceneManager::OnKeyDownCurrentScene(SDL_Keycode key)
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_.at(currentScene_)->OnKeyDown(key);
        }
    }

    void Game::Engine::SceneManager::OnKeyUpCurrentScene(SDL_Keycode key)
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_.at(currentScene_)->OnKeyUp(key);
        }
    }

    void Game::Engine::SceneManager::OnMouseDownCurrentScene(Uint8 button)
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_.at(currentScene_)->OnMouseDown(button);
        }
    }

    void Game::Engine::SceneManager::OnMouseUpCurrentScene(Uint8 button, int clickSpeed)
    {
        if (scenes_.contains(currentScene_))
        {
            scenes_.at(currentScene_)->OnMouseUp(button, clickSpeed);
        }
    }

    Game::Engine::SceneManager::IScene::IScene() //: gui_(std::make_shared<GUI>())
    {
    }

    void Game::Engine::SceneManager::IScene::Initialize()
    {
        InitializeDerived();
    }

    void Game::Engine::SceneManager::IScene::OnEnter()
    {
        OnEnterDerived();
    }

    void Game::Engine::SceneManager::IScene::Update()
    {
        // gui_->Update();

        UpdateDerived();
    }

    void Game::Engine::SceneManager::IScene::Render()
    {
        RenderBeforeGUIDerived();

        // gui_->Render();

        RenderAfterGUIDerived();
    }

    void Game::Engine::SceneManager::IScene::OnKeyDown(SDL_Keycode key)
    {
        // if (gui_->OnKeyDown(key))
        // {
        //     return;
        // }

        OnKeyDownDerived(key);
    }

    void Game::Engine::SceneManager::IScene::OnKeyUp(SDL_Keycode key)
    {
        // if (gui_->OnKeyUp(key))
        // {
        //     return;
        // }

        OnKeyUpDerived(key);
    }

    void Game::Engine::SceneManager::IScene::OnMouseDown(Uint8 button)
    {
        // if (gui_->OnMouseDown(button))
        // {
        //     return;
        // }

        OnMouseDownDerived(button);
    }

    void Game::Engine::SceneManager::IScene::OnMouseUp(Uint8 button, int clickSpeed)
    {
        // if (gui_->OnMouseUp(button, clickSpeed))
        // {
        //     return;
        // }

        OnMouseUpDerived(button, clickSpeed);
    }

    void Game::Engine::Rendering::ImageRenderer::DrawImage(int imageNameHash, float x, float y,
                                                           float width, float height)
    {
        auto &GetCanvasSize = Game::Engine::Common::CanvasUtilities::GetCanvasSize;

        auto &imageBank{Game::Instance().engine_.imageBank_};
        auto &sldDevice{Game::Instance().engine_.sdlDevice_};

        auto canvasSize{GetCanvasSize()};

        auto xPx{static_cast<int>(x * canvasSize.width)};
        auto yPx{static_cast<int>(y * canvasSize.height)};
        auto widthPx{static_cast<int>(width * canvasSize.width)};
        auto heightPx{static_cast<int>(height * canvasSize.height)};

        auto rect{SDL_Rect{xPx, yPx, widthPx, heightPx}};

        auto image{imageBank.GetImage(imageNameHash)};

        SDL_RenderCopy(sldDevice.renderer_.get(), image.get(), nullptr, &rect);
    }

    void Game::Engine::Rendering::ImageRenderer::DrawImage(std::string_view imageName, float x,
                                                           float y, float width, float height)
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto hash{Hash(imageName)};

        DrawImage(hash, x, y, width, height);
    }

    void Game::Engine::Rendering::TextRenderer::Initialize()
    {
        TTF_Init();

        AddFont(FontSizes::_12);
        AddFont(FontSizes::_18);
        AddFont(FontSizes::_24);
    }

    void Game::Engine::Rendering::TextRenderer::AddFont(FontSizes fontSize)
    {
        using SDLDeleter = Game::Engine::Common::SDLDeleter;

        auto &Replace = Game::Engine::Common::StringUtilities::Replace;

        auto absFontPath{std::string(SDL_GetBasePath()) + k_defaultFontPath_};
        auto fontPath{Replace(absFontPath, "\\", "/")};
        auto fontSizeN{static_cast<int>(fontSize)};

        auto newFont{
            std::shared_ptr<TTF_Font>(TTF_OpenFont(fontPath.c_str(), fontSizeN), SDLDeleter())};

        fonts_.insert({fontSize, newFont});
    }

    void Game::Engine::Rendering::TextRenderer::DrawString(std::string_view text, float x, float y,
                                                           FontSizes fontSize, bool centered,
                                                           Common::Matter::Coloring::Color color)
    {
        using SDLDeleter = Game::Engine::Common::SDLDeleter;

        auto &GetCanvasSize = Game::Engine::Common::CanvasUtilities::GetCanvasSize;

        auto &sldDevice{Game::Instance().engine_.sdlDevice_};

        if (text.empty())
        {
            return;
        }

        auto font{fonts_[fontSize]};

        auto sdlColor{color.ToSDLColor()};

        auto surface{std::shared_ptr<SDL_Surface>(
            TTF_RenderText_Solid(font.get(), text.data(), sdlColor), SDLDeleter())};

        auto texture{std::shared_ptr<SDL_Texture>(
            SDL_CreateTextureFromSurface(sldDevice.renderer_.get(), surface.get()), SDLDeleter())};

        auto canvasSize{GetCanvasSize()};

        SDL_Rect rect{static_cast<int>(x * canvasSize.width),
                      static_cast<int>(y * canvasSize.height), surface->w, surface->h};

        if (centered)
        {
            rect.x -= surface->w / 2;
            rect.y -= surface->h / 2;
        }

        SDL_RenderCopy(sldDevice.renderer_.get(), texture.get(), nullptr, &rect);
    }

    void Game::Engine::SceneManager::IntroScene::UpdateDerived()
    {
        // Game::Instance().engine_.sceneManager_.GoToScene("MainMenuScene");
    }

    void Game::Engine::SceneManager::IntroScene::RenderBeforeGUIDerived()
    {
        // std::cout << "IntroScene rendering" << std::endl;

        auto &imageRenderer{Game::Instance().engine_.rendering_.imageRenderer_};

        imageRenderer.DrawImage("DefaultSceneBackground", 0.0f, 0.0f, 1.0f, 1.0f);

        imageRenderer.DrawImage("ForradiaLogo", 0.2f, 0.2f, 0.6f, 0.2f);
    }

    void Game::Engine::SceneManager::MainMenuScene::RenderBeforeGUIDerived()
    {
        // std::cout << "MainMenuScene rendering" << std::endl;
    }
}
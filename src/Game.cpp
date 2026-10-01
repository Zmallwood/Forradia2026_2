/************************************************************************
 *                               Forradia                               *
 *                                                                      *
 * Copyright (c) 2026 Andreas Åkerberg                                  *
 * SPDX-License-Identifier: MIT                                         *
 ************************************************************************/

#include "Game.hpp"

namespace Forradia
{
    Game::Engine::Common::Matter::Geometry::Point
    Game::Engine::Common::Matter::Geometry::Point::operator+(const Point &other) const
    {
        return {x + other.x, y + other.y};
    }

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

    bool Game::Engine::Common::Matter::Geometry::RectF::Contains(
        Game::Engine::Common::Matter::Geometry::PointF point)
    {
        return point.x >= x && point.x <= x + width && point.y >= y && point.y <= y + height;
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

    void Game::Engine::Stop()
    {
        running_ = false;
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

    int Game::Engine::Common::NumberUtilities::InvertSpeed(float speed)
    {
        return static_cast<int>(Game::Engine::Common::Constants::k_oneSecondMillis / speed);
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
        AddScene("WorldGenerationScene", std::make_shared<WorldGenerationScene>());
        AddScene("MainScene", std::make_shared<MainScene>());

        GoToScene("IntroScene");
    }

    void Game::Engine::SceneManager::AddScene(std::string_view sceneName,
                                              std::shared_ptr<IScene> scene)
    {
        auto &Hash = Game::Engine::Common::Hash;

        scenes_.insert({Hash(sceneName), scene});

        scene->Initialize();
    }

    void Game::Engine::SceneManager::GoToScene(std::string_view sceneName)
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto hash{Hash(sceneName)};

        if (scenes_.contains(hash))
        {
            currentScene_ = hash;

            scenes_.at(currentScene_)->OnEnter();
        }
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

    Game::Engine::SceneManager::IScene::GUIComponent::GUIComponent(float x, float y)
        : position_(x, y)
    {
    }

    void Game::Engine::SceneManager::IScene::GUIComponent::Update()
    {
        if (!isEnabled_)
        {
            return;
        }

        if (parent_ && !parent_->isEnabled_)
        {
            return;
        }

        UpdateDerived();

        for (const auto &component : components_)
        {
            component->Update();
        }
    }

    void Game::Engine::SceneManager::IScene::GUIComponent::Render()
    {
        if (!isVisible_ || !isEnabled_)
        {
            return;
        }

        if (parent_ && (!parent_->isVisible_ && !parent_->isEnabled_))
        {
            return;
        }

        RenderDerived();

        for (const auto &component : components_)
        {
            component->Render();
        }
    }

    bool Game::Engine::SceneManager::IScene::GUIComponent::OnMouseDown(Uint8 mouseButton)
    {
        if (!isVisible_ || !isEnabled_)
        {
            return false;
        }

        if (parent_ && (!parent_->isVisible_ && !parent_->isEnabled_))
        {
            return false;
        }

        if (std::any_of(components_.rbegin(), components_.rend(),
                        [=](const std::shared_ptr<GUIComponent> &comp)
                        { return comp->OnMouseDown(mouseButton); }))
        {
            return true;
        }

        return false;
    }

    bool Game::Engine::SceneManager::IScene::GUIComponent::OnMouseUp(Uint8 mouseButton,
                                                                     int clickSpeed)
    {
        if (!isVisible_ || !isEnabled_)
        {
            return false;
        }

        if (parent_ && (!parent_->isVisible_ && !parent_->isEnabled_))
        {
            return false;
        }

        if (std::any_of(components_.rbegin(), components_.rend(),
                        [=](const std::shared_ptr<GUIComponent> &comp)
                        { return comp->OnMouseUp(mouseButton, clickSpeed); }))
        {
            return true;
        }

        return false;
    }

    bool Game::Engine::SceneManager::IScene::GUIComponent::OnKeyDown(SDL_Keycode key)
    {
        if (!isVisible_ || !isEnabled_)
        {
            return false;
        }

        if (parent_ && (!parent_->isVisible_ && !parent_->isEnabled_))
        {
            return false;
        }

        if (std::any_of(components_.rbegin(), components_.rend(),
                        [=](const std::shared_ptr<GUIComponent> &comp)
                        { return comp->OnKeyDown(key); }))
        {
            return true;
        }

        return false;
    }

    bool Game::Engine::SceneManager::IScene::GUIComponent::OnKeyUp(SDL_Keycode key)
    {
        if (!isVisible_ || !isEnabled_)
        {
            return false;
        }

        if (parent_ && (!parent_->isVisible_ && !parent_->isEnabled_))
        {
            return false;
        }

        if (std::any_of(components_.rbegin(), components_.rend(),
                        [=](const std::shared_ptr<GUIComponent> &comp)
                        { return comp->OnKeyUp(key); }))
        {
            return true;
        }

        return false;
    }

    std::shared_ptr<Game::Engine::SceneManager::IScene::GUIComponent>
    Game::Engine::SceneManager::IScene::GUIComponent::AddComponent(
        std::shared_ptr<GUIComponent> component)
    {
        component->parent_ = this;

        components_.push_back(component);

        return component;
    }

    Game::Engine::Common::Matter::Geometry::PointF
    Game::Engine::SceneManager::IScene::GUIComponent::GetPosition()
    {
        using PointF = Game::Engine::Common::Matter::Geometry::PointF;

        PointF finalPosition{0.0F, 0.0F};

        if (parent_)
        {
            finalPosition += parent_->GetPosition();
        }

        finalPosition += position_;

        return finalPosition;
    }

    void Game::Engine::SceneManager::IScene::GUIComponent::SetYPosition(float y)
    {
        position_.y = y;
    }

    Game::Engine::SceneManager::IScene::GUIPanel::GUIPanel(float x, float y, float width,
                                                           float height)
        : GUIComponent(x, y), size_(width, height)
    {
    }

    void Game::Engine::SceneManager::IScene::GUIPanel::RenderDerived()
    {
        auto &ImageRenderer = Game::Instance().engine_.rendering_.imageRenderer_;

        auto position{GetPosition()};

        ImageRenderer.DrawImage(GetBackgroundImage(), position.x, position.y, size_.width,
                                size_.height);
    }

    std::string Game::Engine::SceneManager::IScene::GUIPanel::GetBackgroundImage()
    {
        return k_defaultBackgroundImage_;
    }

    Game::Engine::Common::Matter::Geometry::RectF
    Game::Engine::SceneManager::IScene::GUIPanel::GetBounds()
    {
        auto position{GetPosition()};

        return {position.x, position.y, size_.width, size_.height};
    }

    Game::Engine::SceneManager::IScene::GUIButton::GUIButton(
        std::string_view text, float x, float y, float width, float height,
        std::function<void()> action, std::string_view backgroundImage,
        std::string_view hoveredBackgroundImage)
        : GUIPanel(x, y, width, height), text_(text), action_(action),
          k_backgroundImage_(backgroundImage), k_hoveredBackgroundImage_(hoveredBackgroundImage)
    {
    }

    void Game::Engine::SceneManager::IScene::GUIButton::UpdateDerived()
    {
        GUIPanel::UpdateDerived();

        auto &cursor = Game::Instance().engine_.minorComponents_.cursor_;

        auto &GetMousePosition = Game::Engine::Common::MouseUtilities::GetMousePosition;

        auto position{GetPosition()};

        auto size{size_};

        auto rect{Game::Engine::Common::Matter::Geometry::RectF{position.x, position.y, size.width,
                                                                size.height}};

        if (rect.Contains(GetMousePosition()))
        {
            hovered_ = true;

            cursor.cursorStyle_ = Game::Engine::MinorComponents::Cursor::CursorStyles::Hovering;
        }
        else
        {
            hovered_ = false;
        }
    }

    void Game::Engine::SceneManager::IScene::GUIButton::RenderDerived()
    {
        GUIPanel::RenderDerived();

        auto &textRenderer = Game::Instance().engine_.rendering_.textRenderer_;

        auto position{GetPosition()};

        auto size{size_};

        textRenderer.DrawString(text_, position.x + size.width / 2, position.y + size.height / 2,
                                Game::Engine::Rendering::TextRenderer::FontSizes::_12, true);
    }

    bool Game::Engine::SceneManager::IScene::GUIButton::OnMouseDown(Uint8 mouseButton)
    {
        auto &GetMousePosition = Game::Engine::Common::MouseUtilities::GetMousePosition;

        if (!isVisible_)
        {
            return false;
        }

        auto mousePosition{GetMousePosition()};

        if (GetBounds().Contains(mousePosition))
        {
            action_();

            return true;
        }

        return false;
    }

    std::string Game::Engine::SceneManager::IScene::GUIButton::GetBackgroundImage()
    {
        return hovered_ ? k_hoveredBackgroundImage_ : k_backgroundImage_;
    }

    Game::Engine::SceneManager::IScene::GUIMeter::GUIMeter(float x, float y, float width,
                                                           float height)
        : GUIComponent(x, y), size_(width, height)
    {
    }

    void Game::Engine::SceneManager::IScene::GUIMeter::RenderDerived()
    {
        auto &colorRenderer = Game::Instance().engine_.rendering_.colorRenderer_;

        auto position{GetPosition()};

        auto size{size_};

        colorRenderer.FillRect(position.x, position.y, size.width, size.height,
                               Game::Engine::Common::Matter::Coloring::Colors::k_darkBlue);

        colorRenderer.FillRect(position.x, position.y, GetMeterProgress() * size.width, size.height,
                               GetFilledColor());

        colorRenderer.DrawRect(position.x, position.y, size.width, size.height,
                               Game::Engine::Common::Matter::Coloring::Colors::k_black);
    }

    float Game::Engine::SceneManager::IScene::GUIMeter::GetMeterProgress()
    {
        return 0.0f;
    }

    Game::Engine::Common::Matter::Coloring::Color
    Game::Engine::SceneManager::IScene::GUIMeter::GetFilledColor()
    {
        return Game::Engine::Common::Matter::Coloring::Colors::k_yellowGray;
    }

    Game::Engine::SceneManager::IScene::GUITextConsole::GUITextConsole()
        : GUIPanel(0.0f, 0.8f, 0.4f, 0.2f)
    {
    }

    void Game::Engine::SceneManager::IScene::GUITextConsole::PrintLine(std::string_view line)
    {
        lines_.push_back(line.data());
    }

    void Game::Engine::SceneManager::IScene::GUITextConsole::RenderDerived()
    {
        GUIPanel::RenderDerived();

        auto &textRenderer = Game::Instance().engine_.rendering_.textRenderer_;

        auto position{GetPosition()};

        auto size{size_};

        auto maxNumLines{static_cast<int>(size.height / k_lineHeight_) - 1};

        auto iStart{std::max(0, static_cast<int>(lines_.size() - maxNumLines))};

        auto rowIndex{0};

        for (auto line = iStart; line < lines_.size(); line++)
        {
            if (line > lines_.size() - 1)
                break;

            auto text{lines_[line]};

            textRenderer.DrawString(text, position.x + 0.01f,
                                    position.y + 0.01f + rowIndex * k_lineHeight_);

            rowIndex++;
        }
    }

    Game::Engine::SceneManager::IScene::IScene() : gui_(std::make_shared<GUI>())
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
        gui_->Update();

        UpdateDerived();
    }

    void Game::Engine::SceneManager::IScene::Render()
    {
        RenderBeforeGUIDerived();

        gui_->Render();

        RenderAfterGUIDerived();
    }

    void Game::Engine::SceneManager::IScene::OnKeyDown(SDL_Keycode key)
    {
        if (gui_->OnKeyDown(key))
        {
            return;
        }

        OnKeyDownDerived(key);
    }

    void Game::Engine::SceneManager::IScene::OnKeyUp(SDL_Keycode key)
    {
        if (gui_->OnKeyUp(key))
        {
            return;
        }

        OnKeyUpDerived(key);
    }

    void Game::Engine::SceneManager::IScene::OnMouseDown(Uint8 button)
    {
        if (gui_->OnMouseDown(button))
        {
            return;
        }

        OnMouseDownDerived(button);
    }

    void Game::Engine::SceneManager::IScene::OnMouseUp(Uint8 button, int clickSpeed)
    {
        if (gui_->OnMouseUp(button, clickSpeed))
        {
            return;
        }

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

    void Game::Engine::Rendering::ColorRenderer::FillRect(
        float x, float y, float width, float height,
        Game::Engine::Common::Matter::Coloring::Color color)
    {
        auto &sdlDevice = Game::Instance().engine_.sdlDevice_;

        auto rect{CreateSDLRect(x, y, width, height)};

        auto sdlColor{color.ToSDLColor()};

        SDL_SetRenderDrawColor(sdlDevice.renderer_.get(), sdlColor.r, sdlColor.g, sdlColor.b,
                               sdlColor.a);

        SDL_RenderFillRect(sdlDevice.renderer_.get(), &rect);
    }

    void Game::Engine::Rendering::ColorRenderer::DrawRect(
        float x, float y, float width, float height,
        Game::Engine::Common::Matter::Coloring::Color color)
    {
        auto &sdlDevice = Game::Instance().engine_.sdlDevice_;

        auto rect{CreateSDLRect(x, y, width, height)};

        auto sdlColor{color.ToSDLColor()};

        SDL_SetRenderDrawColor(sdlDevice.renderer_.get(), sdlColor.r, sdlColor.g, sdlColor.b,
                               sdlColor.a);

        SDL_RenderDrawRect(sdlDevice.renderer_.get(), &rect);
    }

    void Game::Engine::Rendering::ColorRenderer::DrawLine(
        float x1, float y1, float x2, float y2, Game::Engine::Common::Matter::Coloring::Color color)
    {
        auto &GetCanvasSize = Game::Engine::Common::CanvasUtilities::GetCanvasSize;

        auto &sdlDevice = Game::Instance().engine_.sdlDevice_;

        auto canvasSize{GetCanvasSize()};

        auto destX1{static_cast<int>(x1 * canvasSize.width)};
        auto destY1{static_cast<int>(y1 * canvasSize.height)};
        auto destX2{static_cast<int>(x2 * canvasSize.width)};
        auto destY2{static_cast<int>(y2 * canvasSize.height)};

        auto sdlColor{color.ToSDLColor()};

        SDL_SetRenderDrawColor(sdlDevice.renderer_.get(), sdlColor.r, sdlColor.g, sdlColor.b,
                               sdlColor.a);

        SDL_RenderDrawLine(sdlDevice.renderer_.get(), destX1, destY1, destX2, destY2);
    }

    SDL_Rect Game::Engine::Rendering::ColorRenderer::CreateSDLRect(float x, float y, float width,
                                                                   float height)
    {
        auto &GetCanvasSize = Game::Engine::Common::CanvasUtilities::GetCanvasSize;

        auto canvasSize{GetCanvasSize()};

        auto destX{static_cast<int>(std::floor(x * canvasSize.width))};
        auto destY{static_cast<int>(std::floor(y * canvasSize.height))};
        auto destWidth{static_cast<int>(std::ceil(width * canvasSize.width))};
        auto destHeight{static_cast<int>(std::ceil(height * canvasSize.height))};

        return SDL_Rect{destX, destY, destWidth, destHeight};
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

    Game::Engine::World::World() : currentWorldArea_(std::make_shared<WorldArea>())
    {
    }

    Game::Engine::World::WorldArea::WorldArea()
    {
        auto worldAreaSize{Game::Engine::Configuration::GameProperties::k_worldAreaSize_};

        for (auto x = 0; x < worldAreaSize.width; x++)
        {
            tiles_.push_back(std::vector<std::shared_ptr<Tile>>());

            for (auto y = 0; y < worldAreaSize.height; y++)
            {
                tiles_.at(x).push_back(std::make_shared<Tile>());
            }
        }
    }

    Game::Engine::Common::Matter::Geometry::Size Game::Engine::World::WorldArea::GetSize()
    {
        auto width{static_cast<int>(tiles_.size())};
        auto height{0};

        if (width)
        {
            height = static_cast<int>(tiles_.at(0).size());
        }

        return {width, height};
    }

    bool Game::Engine::World::WorldArea::IsValidCoordinate(int x, int y)
    {
        auto size{GetSize()};

        return x >= 0 && x < size.width && y >= 0 && y < size.height;
    }

    bool
    Game::Engine::World::WorldArea::IsValidCoordinate(Common::Matter::Geometry::Point coordinate)
    {
        return IsValidCoordinate(coordinate.x, coordinate.y);
    }

    std::shared_ptr<Game::Engine::World::WorldArea::Tile>
    Game::Engine::World::WorldArea::GetTile(int x, int y)
    {
        if (IsValidCoordinate(x, y))
        {
            return tiles_.at(x).at(y);
        }

        return nullptr;
    }

    std::shared_ptr<Game::Engine::World::WorldArea::Tile>
    Game::Engine::World::WorldArea::GetTile(Common::Matter::Geometry::Point coordinate)
    {
        return GetTile(coordinate.x, coordinate.y);
    }

    Game::Engine::World::WorldArea::Tile::Tile() : tileObjects_(std::make_shared<TileObjects>())
    {
    }

    void Game::Engine::World::WorldArea::Tile::TileObjects::Clear()
    {
        objects_.clear();
    }

    void Game::Engine::World::WorldArea::Tile::TileObjects::AddObject(
        int objectType, Common::Matter::Geometry::Point position)
    {
        auto &gameProperties = Game::Instance().engine_.configuration_.gameProperties_;

        if (position.x == -1 || position.y == -1)
        {
            position.x = rand() % gameProperties.k_tileUnitsWidth_;
            position.y = rand() % gameProperties.k_tileUnitsWidth_;
        }

        objects_.insert({position, std::make_shared<Object>(objectType)});
    }

    void Game::Engine::World::WorldArea::Tile::TileObjects::AddObject(
        std::string_view objectName, Common::Matter::Geometry::Point position)
    {
        AddObject(Game::Engine::Common::Hash(objectName), position);
    }

    void Game::Engine::World::WorldArea::Tile::TileObjects::AddObject(
        std::shared_ptr<Object> object, Common::Matter::Geometry::Point position)
    {
        objects_.insert({position, object});
    }

    int Game::Engine::World::WorldArea::Tile::TileObjects::Count()
    {
        return objects_.size();
    }

    std::shared_ptr<Game::Engine::World::WorldArea::Tile::TileObjects::Object>
    Game::Engine::World::WorldArea::Tile::TileObjects::PickObject(
        std::shared_ptr<Game::Engine::World::WorldArea::Tile::TileObjects::Object> object)
    {
        for (auto it = objects_.begin(); it != objects_.end(); ++it)
        {
            if (it->second == object)
            {
                objects_.erase(it);

                return object;
            }
        }

        return nullptr;
    }

    Game::Engine::World::WorldArea::Tile::TileObjects::Object::Object(int type) : type_(type)
    {
    }

    Game::Engine::World::WorldArea::Tile::TileObjects::Object::Object(std::string_view typeName)
        : type_(Game::Engine::Common::Hash(typeName))
    {
    }

    Game::Engine::World::WorldArea::Tile::Creature::Creature(std::string_view typeName)
    {
        type_ = Game::Engine::Common::Hash(typeName);

        corpesType_ = Game::Engine::Common::Hash("Object" + std::string(typeName) + "Corpse");
    }

    Game::Engine::World::WorldArea::Tile::Creature::Creature(int type)
    {
        type_ = type;

        // auto typeName{_<CreatureIndex>().GetCreatureLabel(type)};

        // corpesType_ = Hash("Object" + std::string(typeName) + "Corpse");
    }

    void Game::Engine::World::WorldArea::Tile::Creature::Hit(
        float damage, Common::Matter::Geometry::PointF hitPosition)
    {
        auto &Now{Game::Engine::Common::TimeUtilities::Now};

        auto &InvertSpeed{Game::Engine::Common::NumberUtilities::InvertSpeed};

        health_ -= damage;

        ticksLastHitOnSelf_ = Now();

        lastHitPosition_ = hitPosition;

        targetingPlayer_ = true;

        auto now{Now()};

        if (now - ticksLastHitOnOther_ > InvertSpeed(attackSpeed_))
        {
            ticksLastHitOnOther_ = now;
        }

        // auto creatureLabel = _<CreatureIndex>().GetCreatureLabel(type_);

        // std::stringstream ssDamage;
        // ssDamage << std::fixed << std::setprecision(1) << damage;

        // _<GUITextConsole>().PrintLine("You hit a " + creatureLabel + " for " + ssDamage.str() +
        //                               " damage.");
    }

    bool Game::Engine::World::WorldArea::Tile::Creature::IsDead()
    {
        return health_ <= 0.0f;
    }

    void Game::Engine::SceneManager::IntroScene::RenderBeforeGUIDerived()
    {
        auto &Now{Game::Engine::Common::TimeUtilities::Now};

        auto &imageRenderer{Game::Instance().engine_.rendering_.imageRenderer_};

        auto &textRenderer{Game::Instance().engine_.rendering_.textRenderer_};

        imageRenderer.DrawImage("DefaultSceneBackground", 0.0f, 0.0f, 1.0f, 1.0f);

        imageRenderer.DrawImage("ForradiaLogo", 0.2f, 0.2f, 0.6f, 0.2f);

        if (Now() % 800 < 400)
        {
            textRenderer.DrawString("Press to start", 0.5f, 0.5f,
                                    Game::Engine::Rendering::TextRenderer::FontSizes::_24, true);
        }
    }

    void Game::Engine::SceneManager::IntroScene::OnKeyDownDerived(SDL_Keycode key)
    {
        Game::Instance().engine_.sceneManager_.GoToScene("MainMenuScene");
    }

    void Game::Engine::SceneManager::IntroScene::OnMouseDownDerived(Uint8 button)
    {
        Game::Instance().engine_.sceneManager_.GoToScene("MainMenuScene");
    }

    void Game::Engine::SceneManager::MainMenuScene::InitializeDerived()
    {
        gui_->AddComponent(GUITextConsole::InstancePtr());

        gui_->AddComponent(std::make_shared<GUIPanel>(0.4f, 0.4f, 0.2f, 0.2f));

        gui_->AddComponent(std::make_shared<GUIButton>(
            "Play", 0.45f, 0.44f, 0.1f, 0.04f, [this]()
            { Game::Instance().engine_.sceneManager_.GoToScene("WorldGenerationScene"); }));

        gui_->AddComponent(std::make_shared<GUIButton>("Quit ", 0.45f, 0.52f, 0.1f, 0.04f, [this]()
                                                       { Game::Instance().engine_.Stop(); }));
    }

    void Game::Engine::SceneManager::MainMenuScene::OnEnterDerived()
    {
        GUITextConsole::InstancePtr()->PrintLine("Starting game.");
    }

    void Game::Engine::SceneManager::MainMenuScene::RenderBeforeGUIDerived()
    {
        auto &imageRenderer{Game::Instance().engine_.rendering_.imageRenderer_};

        imageRenderer.DrawImage("DefaultSceneBackground", 0.0f, 0.0f, 1.0f, 1.0f);

        imageRenderer.DrawImage("ForradiaLogo", 0.35f, 0.15f, 0.3f, 0.15f);
    }

    void Game::Engine::SceneManager::WorldGenerationScene::OnEnterDerived()
    {
        worldGenerator_.GenerateNewWorld();

        Game::Instance().engine_.sceneManager_.GoToScene("MainScene");
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateNewWorld()
    {
        ClearWithGrass();

        GenerateDirt();

        GenerateWater();

        GenerateElevation();

        GenerateRock();

        GenerateLargeObjects();

        GenerateSmallObjects();

        GenerateCreatures();
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::ClearWithGrass()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        for (auto y = 0; y < size.height; y++)
        {
            for (auto x = 0; x < size.width; x++)
            {
                auto tile{worldArea->GetTile(x, y)};

                tile->ground_ = Hash("GroundGrass");
            }
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateDirt()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numDirtPatches{15 + rand() % 8};

        for (auto i = 0; i < numDirtPatches; i++)
        {
            auto xCenter{rand() % size.width};
            auto yCenter{rand() % size.height};
            auto radius{3 + rand() % 14};

            for (auto y = yCenter - radius; y <= yCenter + radius; y++)
            {
                for (auto x = xCenter - radius; x <= xCenter + radius; x++)
                {
                    if (!worldArea->IsValidCoordinate(x, y))
                    {
                        continue;
                    }

                    auto dx{x - xCenter};
                    auto dy{y - yCenter};

                    if (dx * dx + dy * dy <= radius * radius)
                    {
                        auto tile{worldArea->GetTile(x, y)};

                        tile->ground_ = Hash("GroundDirt");
                    }
                }
            }
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateWater()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numLakes{40 + rand() % 20};

        for (auto i = 0; i < numLakes; i++)
        {
            auto xCenter{rand() % size.width};
            auto yCenter{rand() % size.height};
            auto radius{3 + rand() % 6};

            for (auto y = yCenter - radius; y <= yCenter + radius; y++)
            {
                for (auto x = xCenter - radius; x <= xCenter + radius; x++)
                {
                    if (!worldArea->IsValidCoordinate(x, y))
                    {
                        continue;
                    }

                    auto dx{x - xCenter};
                    auto dy{y - yCenter};

                    if (dx * dx + dy * dy <= radius * radius)
                    {
                        auto tile{worldArea->GetTile(x, y)};

                        tile->ground_ = Hash("GroundWater");
                    }
                }
            }
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateElevation()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        for (auto y = 0; y < size.height; y++)
        {
            for (auto x = 0; x < size.width; x++)
            {
                auto tile{worldArea->GetTile(x, y)};

                if (tile->ground_ != Hash("GroundWater"))
                {
                    tile->elevation_ = 1;
                }
            }
        }

        auto numHills{20 + rand() % 10};

        for (auto i = 0; i < numHills; i++)
        {
            auto xCenter{rand() % size.width};
            auto yCenter{rand() % size.height};
            auto radius{3 + rand() % 9};

            for (auto r = radius; r >= 0; r--)
            {
                for (auto y = yCenter - r; y <= yCenter + r; y++)
                {
                    for (auto x = xCenter - r; x <= xCenter + r; x++)
                    {
                        if (!worldArea->IsValidCoordinate(x, y))
                        {
                            continue;
                        }

                        auto dx{x - xCenter};
                        auto dy{y - yCenter};

                        if (dx * dx + dy * dy <= r * r)
                        {
                            auto tile{worldArea->GetTile(x, y)};

                            if (tile->ground_ == Hash("GroundWater"))
                            {
                                continue;
                            }

                            ++tile->elevation_;
                        }
                    }
                }
            }
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateRock()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numRockPatches{60 + rand() % 5};

        for (auto i = 0; i < numRockPatches; i++)
        {
            auto xCenter{rand() % size.width};
            auto yCenter{rand() % size.height};
            auto radius{3 + rand() % 9};

            for (auto y = yCenter - radius; y <= yCenter + radius; y++)
            {
                for (auto x = xCenter - radius; x <= xCenter + radius; x++)
                {
                    if (!worldArea->IsValidCoordinate(x, y))
                    {
                        continue;
                    }

                    auto dx{x - xCenter};
                    auto dy{y - yCenter};

                    if (dx * dx + dy * dy <= radius * radius)
                    {
                        auto tile{worldArea->GetTile(x, y)};

                        if (tile->elevation_ >= 2)
                        {
                            tile->ground_ = Hash("GroundRock");
                        }
                    }
                }
            }
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateLargeObjects()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numTree2Groups{100 + rand() % 10};

        for (auto i = 0; i < numTree2Groups; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto numTree2s{60 + rand() % 10};

            for (auto j = 0; j < numTree2s; j++)
            {
                x += rand() % 3 - rand() % 3;
                y += rand() % 3 - rand() % 3;

                if (!worldArea->IsValidCoordinate(x, y))
                {
                    continue;
                }

                auto tile{worldArea->GetTile(x, y)};

                if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
                {
                    continue;
                }

                tile->tileObjects_->Clear();

                tile->tileObjects_->AddObject("ObjectTree2");
            }
        }

        auto numTree1Groups{100 + rand() % 10};

        for (auto i = 0; i < numTree1Groups; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto numTree1s{60 + rand() % 10};

            for (auto j = 0; j < numTree1s; j++)
            {
                x += rand() % 3 - rand() % 3;
                y += rand() % 3 - rand() % 3;

                if (!worldArea->IsValidCoordinate(x, y))
                {
                    continue;
                }

                auto tile{worldArea->GetTile(x, y)};

                if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
                {
                    continue;
                }

                tile->tileObjects_->Clear();

                tile->tileObjects_->AddObject("ObjectTree1");
            }
        }

        auto numBush1s{300 + rand() % 50};

        for (auto i = 0; i < numBush1s; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundDirt") ||
                tile->ground_ == Hash("GroundRock"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectBush1");
        }

        auto numStoneBoulders{100 + rand() % 50};

        for (auto i = 0; i < numStoneBoulders; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundGrass") || tile->ground_ == Hash("GroundDirt"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectStoneBoulder");
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateSmallObjects()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numStones{500 + rand() % 50};

        for (auto i = 0; i < numStones; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectStone");
        }

        auto numBranches{500 + rand() % 50};

        for (auto i = 0; i < numBranches; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectBranch");
        }

        auto numPinkFlowers{500 + rand() % 50};

        for (auto i = 0; i < numPinkFlowers; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock") ||
                tile->ground_ == Hash("GroundDirt"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectPinkFlower");
        }

        auto numLeaves{500 + rand() % 50};

        for (auto i = 0; i < numLeaves; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
            {
                continue;
            }

            tile->tileObjects_->AddObject("ObjectLeaf");
        }
    }

    void Game::Engine::SceneManager::WorldGenerationScene::WorldGenerator::GenerateCreatures()
    {
        auto &Hash = Game::Engine::Common::Hash;

        auto &world = Game::Instance().engine_.world_;

        using Creature = Game::Engine::World::WorldArea::Tile::Creature;

        auto worldArea{world.currentWorldArea_};
        auto size{worldArea->GetSize()};

        auto numDeers{150 + rand() % 20};

        for (auto i = 0; i < numDeers; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
            {
                continue;
            }

            auto newCreature{std::make_shared<Creature>("CreatureDeer")};

            worldArea->creaturesMirror_.insert({newCreature, {x, y}});

            tile->creature_ = newCreature;
        }

        auto numBoars{150 + rand() % 20};

        for (auto i = 0; i < numBoars; i++)
        {
            auto x{rand() % size.width};
            auto y{rand() % size.height};

            auto tile{worldArea->GetTile(x, y)};

            if (tile->ground_ == Hash("GroundWater") || tile->ground_ == Hash("GroundRock"))
            {
                continue;
            }

            auto newCreature{std::make_shared<Creature>("CreatureBoar")};

            worldArea->creaturesMirror_.insert({newCreature, {x, y}});
        }
    }
}
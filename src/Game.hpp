/************************************************************************
 *                               Forradia                               *
 *                                                                      *
 * Copyright (c) 2026 Andreas Åkerberg                                  *
 * SPDX-License-Identifier: MIT                                         *
 ************************************************************************/

#pragma once

namespace Forradia
{
    class Game
    {
      public:
        static Game &Instance()
        {
            static Game instance;

            return instance;
        }

        Game(const Game &) = delete;

        Game &operator=(const Game &) = delete;

        void Start();

      private:
        Game() = default;

        class Engine
        {
          public:
            void Run();

            void Stop();

          private:
            class Common
            {
              public:
                class Constants
                {
                  public:
                    static constexpr float k_smallValue{0.0005f};
                    static constexpr int k_oneSecondMillis{1000};
                };

                class Matter
                {
                  public:
                    class Geometry
                    {
                      public:
                        class Point
                        {
                          public:
                            auto operator<=>(const Point &) const = default;

                            Point operator+(const Point &other) const;

                            int x{0};
                            int y{0};
                        };

                        class Size
                        {
                          public:
                            int width{0};
                            int height{0};
                        };

                        class PointF
                        {
                          public:
                            void operator+=(const PointF &other);

                            PointF operator+(const PointF &other) const;

                            PointF operator-(const PointF &other) const;

                            float x{0.0f};
                            float y{0.0f};
                        };

                        class SizeF
                        {
                          public:
                            float width{0.0f};
                            float height{0.0f};
                        };

                        class RectF
                        {
                          public:
                            bool Contains(PointF point);

                            float x{0.0f};
                            float y{0.0f};
                            float width{0.0f};
                            float height{0.0f};
                        };
                    };

                    class Coloring
                    {
                      public:
                        class Color
                        {
                          public:
                            SDL_Color ToSDLColor();

                            float r{0.0f};
                            float g{0.0f};
                            float b{0.0f};
                            float a{1.0f};
                        };

                        class Colors
                        {
                          public:
                            static constexpr Color k_black{0.0f, 0.0f, 0.0f, 1.0f};
                            static constexpr Color k_white{1.0f, 1.0f, 1.0f, 1.0f};
                            static constexpr Color k_gold{1.0f, 0.84f, 0.0f, 1.0f};
                            static constexpr Color k_wheat{0.96f, 0.87f, 0.7f, 1.0f};
                            static constexpr Color k_darkBlue{0.0f, 0.0f, 0.3f, 1.0f};
                            static constexpr Color k_yellowGray{0.85f, 0.75f, 0.4f, 1.0f};
                            static constexpr Color k_red{0.8f, 0.0f, 0.0f, 1.0f};
                        };
                    };
                };

                class SDLDeleter
                {
                  public:
                    void operator()(SDL_Window *window);

                    void operator()(SDL_Renderer *renderer);

                    void operator()(SDL_Surface *surface);

                    void operator()(SDL_Texture *texture);

                    void operator()(TTF_Font *font);
                };

                static constexpr auto Hash(std::string_view text) -> int
                {
                    // Use djb2 algorithm by Daniel J. Bernstein.
                    unsigned long hash{5381};

                    for (char chr : text)
                    {
                        constexpr unsigned long algorithmFactor{33};

                        hash = algorithmFactor * hash + static_cast<unsigned char>(chr);
                    }

                    return static_cast<int>(hash);
                }

                class StringUtilities
                {
                  public:
                    static std::string Replace(std::string_view text, std::string_view oldValue,
                                               std::string_view newValue);
                };

                class FilePathUtilities
                {
                  public:
                    static std::string GetFileNameNoExt(std::string_view path);
                };

                class CanvasUtilities
                {
                  public:
                    static Common::Matter::Geometry::Size GetCanvasSize();

                    static float GetAspectRatio();

                    static float ConvertWidthToHeight(float width);

                    static float ConvertHeightToWidth(float height);
                };

                class TimeUtilities
                {
                  public:
                    static int Now();
                };

                class MouseUtilities
                {
                  public:
                    static Common::Matter::Geometry::PointF GetMousePosition();
                };

                class NumberUtilities
                {
                  public:
                    static int InvertSpeed(float speed);
                };
            };

            class SDLDevice
            {
              public:
                void Initialize();

                void ClearCanvas();

                void PresentCanvas();

                void Clip(float x, float y, float width, float height);

                void ResetClip();

                std::shared_ptr<SDL_Window> window_;
                std::shared_ptr<SDL_Renderer> renderer_;

              private:
                static constexpr std::string_view k_windowName_{"Forradia"};
            } sdlDevice_;

            class ImageBank
            {
              public:
                void LoadImages();

                std::shared_ptr<SDL_Texture> GetImage(int imageNameHash);

                Common::Matter::Geometry::Size GetImageSize(int imageNameHash);

                bool IsPixelVisible(int imageNameHash, float x, float y);

              private:
                class ImageEntry
                {
                  public:
                    std::shared_ptr<SDL_Texture> texture;
                    std::shared_ptr<SDL_Surface> surface;
                };

                void LoadSingleImage(std::string_view fullPath);

                static constexpr std::string_view k_relativeImagesDirectory_{"resources/Images/"};
                std::unordered_map<int, ImageEntry> images_;
            } imageBank_;

            class Rendering
            {
              public:
                class ImageRenderer
                {
                  public:
                    void DrawImage(int imageNameHash, float x, float y, float width, float height);

                    void DrawImage(std::string_view imageName, float x, float y, float width, float height);
                } imageRenderer_;

                class ColorRenderer
                {
                  public:
                    void FillRect(float x, float y, float width, float height,
                                  Common::Matter::Coloring::Color color = Common::Matter::Coloring::Colors::k_black);

                    void DrawRect(float x, float y, float width, float height,
                                  Common::Matter::Coloring::Color color = Common::Matter::Coloring::Colors::k_black);

                    void DrawLine(float x1, float y1, float x2, float y2,
                                  Common::Matter::Coloring::Color color = Common::Matter::Coloring::Colors::k_black);

                  private:
                    SDL_Rect CreateSDLRect(float x, float y, float width, float height);
                } colorRenderer_;

                class TextRenderer
                {
                  public:
                    enum class FontSizes : int
                    {
                        _12 = 12,
                        _18 = 18,
                        _24 = 24,
                    };

                    void Initialize();

                    void DrawString(std::string_view text, float x, float y, FontSizes fontSize = FontSizes::_12,
                                    bool centered = false,
                                    Common::Matter::Coloring::Color color = Common::Matter::Coloring::Colors::k_wheat);

                  private:
                    void AddFont(FontSizes fontSize);

                    const std::string k_defaultFontPath_{"./resources/Fonts/PixeloidSans.ttf"};
                    std::unordered_map<FontSizes, std::shared_ptr<TTF_Font>> fonts_;
                } textRenderer_;
            } rendering_;

            class MinorComponents
            {
              public:
                class FPSCounter
                {
                  public:
                    void Update();

                    void Render();

                  private:
                    int fps_{0};
                    int framesCount_{0};
                    int ticksLastUpdate_{0};
                } fpsCounter_;

                class Cursor
                {
                  public:
                    enum class CursorStyles
                    {
                        Default,
                        Hovering
                    };

                    Cursor();

                    void Reset();

                    void Render();

                    CursorStyles cursorStyle_{CursorStyles::Default};

                  private:
                    static constexpr float k_cursorSize_{0.05f};
                } cursor_;
            } minorComponents_;

            class Configuration
            {
              public:
                class GameProperties
                {
                  public:
                    static constexpr Common::Matter::Geometry::Size k_worldAreaSize_{100, 100};
                    static constexpr float k_tileWidth_{0.05f};
                    static constexpr int k_tileUnitsWidth_{20};
                    static constexpr float k_viewWidth_{0.5f};
                    static constexpr float k_largeObjectScale_{0.22f};
                    static constexpr float k_smallObjectScale_{0.08f};
                    static constexpr Common::Matter::Geometry::PointF k_firstPersonViewMargin_{0.03f, 0.01f};
                } gameProperties_;
            } configuration_;

            class World
            {
              public:
                enum class WorldDirections
                {
                    North,
                    East,
                    South,
                    West,
                };

                class WorldArea
                {
                  public:
                    class Tile
                    {
                      public:
                        class TileObjects
                        {
                          public:
                            class Object
                            {
                              public:
                                Object(int type);

                                Object(std::string_view typeName);

                                int type_{0};
                            };

                            void Clear();

                            void AddObject(int objectType, Common::Matter::Geometry::Point position = {-1, -1});

                            void AddObject(std::string_view objectName,
                                           Common::Matter::Geometry::Point position = {-1, -1});

                            void AddObject(std::shared_ptr<Object> object,
                                           Common::Matter::Geometry::Point position = {-1, -1});

                            int Count();

                            std::shared_ptr<Object> PickObject(std::shared_ptr<Object> object);

                            std::map<Common::Matter::Geometry::Point, std::shared_ptr<Object>> objects_;
                        };

                        class Creature
                        {
                          public:
                            Creature(std::string_view typeName);

                            Creature(int type);

                            void Hit(float damage, Common::Matter::Geometry::PointF hitPosition);

                            bool IsDead();

                            int type_{0};
                            int corpesType_{0};
                            int ticksLastMovement_{0};
                            float movementSpeed_{1.0f};
                            int ticksLastHitOnSelf_{0};
                            Common::Matter::Geometry::PointF lastHitPosition_{-1.0f, -1.0f};
                            int experienceValue_{13};
                            int respawnTimeMillis_{5000};
                            int ticksLastHitOnOther_{0};
                            float attackSpeed_{0.5f};
                            bool targetingPlayer_{false};

                          private:
                            float health_{5.0f};
                            float maxHealth_{5.0f};
                        };

                        Tile();

                        int ground_{0};
                        int elevation_{0};
                        std::shared_ptr<TileObjects> tileObjects_;
                        std::shared_ptr<Creature> creature_;
                    };

                    WorldArea();

                    Common::Matter::Geometry::Size GetSize();

                    bool IsValidCoordinate(int x, int y);

                    bool IsValidCoordinate(Common::Matter::Geometry::Point coordinate);

                    std::shared_ptr<Tile> GetTile(int x, int y);

                    std::shared_ptr<Tile> GetTile(Common::Matter::Geometry::Point coordinate);

                    std::unordered_map<std::shared_ptr<Tile::Creature>, Common::Matter::Geometry::Point>
                        creaturesMirror_;

                  private:
                    std::vector<std::vector<std::shared_ptr<Tile>>> tiles_;
                };

                World();

                std::shared_ptr<WorldArea> currentWorldArea_;
            } world_;

            class CoreGameObjects
            {
              public:
                class Player
                {
                  public:
                    class PlayerInventory
                    {
                      public:
                        void Clear();

                        void AddObject(std::string_view objectName);

                        void AddObject(std::shared_ptr<World::WorldArea::Tile::TileObjects::Object> object,
                                       int slotIndex);

                        std::shared_ptr<World::WorldArea::Tile::TileObjects::Object> GetObject(int index);

                        std::shared_ptr<World::WorldArea::Tile::TileObjects::Object> PickObject(int index);

                        bool HasObject(int index);

                      private:
                        static constexpr int k_maxObjects_{1000};
                        std::unordered_map<int, std::shared_ptr<World::WorldArea::Tile::TileObjects::Object>> objects_;
                    } playerInventory_;

                    void Initialize();

                    void MoveNorth();

                    void MoveEast();

                    void MoveSouth();

                    void MoveWest();

                    void TurnNorth();

                    void TurnEast();

                    void TurnSouth();

                    void TurnWest();

                    void AddExperience(int amount);

                    void Hit(float damage);

                    Common::Matter::Geometry::Point position_{0, 0};
                    int ticksLastMovement_{0};
                    float movementSpeed_{4.0f};
                    Common::Matter::Geometry::Point destination_{-1, -1};
                    Common::Matter::Geometry::Point facedTileCoordinate_{-1, -1};
                    World::WorldDirections facingDirection_{World::WorldDirections::South};
                    int ticksLastHitOnOther_{0};
                    float attackSpeed_{2.0f};
                    std::string name_{"Unnamed player"};
                    int experience_{0};
                    float health_{10.0f};
                    float maxHealth_{10.0f};
                    int ticksLastHitOnSelf_{0};

                  private:
                    void SpawnOnSuitableLocation();
                } player_;
            } coreGameObjects_;

            class SceneManager
            {
              public:
                class IScene
                {
                  public:
                    class GUIComponent
                    {
                      public:
                        GUIComponent() = default;

                        GUIComponent(float x, float y);

                        void Update();

                        void Render();

                        virtual bool OnMouseDown(Uint8 mouseButton);

                        virtual bool OnMouseUp(Uint8 mouseButton, int clickSpeed);

                        virtual bool OnKeyDown(SDL_Keycode key);

                        virtual bool OnKeyUp(SDL_Keycode key);

                        std::shared_ptr<GUIComponent> AddComponent(std::shared_ptr<GUIComponent> component);

                        virtual Common::Matter::Geometry::PointF GetPosition();

                        void SetYPosition(float y);

                        virtual void SetPosition(Common::Matter::Geometry::PointF value)
                        {
                            position_ = value;
                        }

                        bool isVisible_{true};
                        bool isEnabled_{true};

                      protected:
                        virtual void UpdateDerived()
                        {
                        }

                        virtual void RenderDerived()
                        {
                        }

                      private:
                        std::vector<std::shared_ptr<GUIComponent>> components_;
                        Common::Matter::Geometry::PointF position_{0.0f, 0.0f};
                        GUIComponent *parent_{nullptr};
                    };

                    class GUI : public GUIComponent
                    {
                      public:
                        using GUIComponent::GUIComponent;
                    };

                    class GUIPanel : public GUIComponent
                    {
                      public:
                        GUIPanel(float x, float y, float width, float height);

                        Common::Matter::Geometry::SizeF size_{0.0f, 0.0f};

                      protected:
                        virtual void RenderDerived() override;

                        virtual std::string GetBackgroundImage();

                        Common::Matter::Geometry::RectF GetBounds();

                      private:
                        inline static const std::string k_defaultBackgroundImage_{"GUIPanelBackground"};
                    };

                    class GUIButton : public GUIPanel
                    {
                      public:
                        GUIButton(std::string_view text, float x, float y, float width, float height,
                                  std::function<void()> action,
                                  std::string_view backgroundImage = "GUIButtonBackground",
                                  std::string_view hoveredBackgroundImage = "GUIButtonHoveredBackground");

                      protected:
                        void UpdateDerived() override;

                        void RenderDerived() override;

                        bool OnMouseDown(Uint8 mouseButton) override;

                        std::string GetBackgroundImage() override;

                      private:
                        const std::string k_backgroundImage_{"GUIButtonBackground"};
                        const std::string k_hoveredBackgroundImage_{"GUIButtonHoveredBackground"};

                        std::string text_;
                        std::function<void()> action_;
                        bool hovered_{false};
                    };

                    class GUIMeter : public GUIComponent
                    {
                      public:
                        GUIMeter(float x, float y, float width, float height);

                        Common::Matter::Geometry::SizeF size_;

                      protected:
                        virtual void RenderDerived() override;

                        virtual float GetMeterProgress();

                        virtual Common::Matter::Coloring::Color GetFilledColor();
                    };

                    class GUITextConsole : public GUIPanel
                    {
                      public:
                        static std::shared_ptr<GUITextConsole> InstancePtr()
                        {
                            static std::shared_ptr<GUITextConsole> instancePtr{new GUITextConsole()};

                            return instancePtr;
                        }

                        GUITextConsole(const GUITextConsole &) = delete;

                        GUITextConsole &operator=(const GUITextConsole &) = delete;

                        void PrintLine(std::string_view line);

                      protected:
                        void RenderDerived() override;

                      private:
                        GUITextConsole();

                        static constexpr float k_lineHeight_{0.02f};

                        std::vector<std::string> lines_;
                    };

                    IScene();

                    void Initialize();

                    void Update();

                    void Render();

                    void OnEnter();

                    void OnKeyDown(SDL_Keycode key);

                    void OnKeyUp(SDL_Keycode key);

                    void OnMouseDown(Uint8 button);

                    void OnMouseUp(Uint8 button, int clickSpeed);

                  protected:
                    virtual void InitializeDerived()
                    {
                    }

                    virtual void OnEnterDerived()
                    {
                    }

                    virtual void UpdateDerived()
                    {
                    }

                    virtual void RenderBeforeGUIDerived()
                    {
                    }

                    virtual void RenderAfterGUIDerived()
                    {
                    }

                    virtual void OnKeyDownDerived(SDL_Keycode key)
                    {
                    }

                    virtual void OnKeyUpDerived(SDL_Keycode key)
                    {
                    }

                    virtual void OnMouseDownDerived(Uint8 button)
                    {
                    }

                    virtual void OnMouseUpDerived(Uint8 button, int clickSpeed)
                    {
                    }

                    std::shared_ptr<GUI> gui_;
                };

                void Initialize();

                void GoToScene(std::string_view sceneName);

                void UpdateCurrentScene();

                void RenderCurrentScene();

                void OnKeyDownCurrentScene(SDL_Keycode key);

                void OnKeyUpCurrentScene(SDL_Keycode key);

                void OnMouseDownCurrentScene(Uint8 button);

                void OnMouseUpCurrentScene(Uint8 button, int clickSpeed);

              private:
                class IntroScene : public IScene
                {
                  protected:
                    void RenderBeforeGUIDerived() override;

                    void OnKeyDownDerived(SDL_Keycode key) override;

                    void OnMouseDownDerived(Uint8 button) override;
                };

                class MainMenuScene : public IScene
                {
                  protected:
                    void InitializeDerived() override;

                    void OnEnterDerived() override;

                    void RenderBeforeGUIDerived() override;
                };

                class WorldGenerationScene : public IScene
                {
                  protected:
                    void OnEnterDerived() override;

                  private:
                    class WorldGenerator
                    {
                      public:
                        void GenerateNewWorld();

                      private:
                        void ClearWithGrass();

                        void GenerateDirt();

                        void GenerateWater();

                        void GenerateElevation();

                        void GenerateRock();

                        void GenerateLargeObjects();

                        void GenerateSmallObjects();

                        void GenerateCreatures();
                    } worldGenerator_;
                };

                class MainScene : public IScene
                {
                  protected:
                    void InitializeDerived() override;

                    void OnEnterDerived() override;

                    void UpdateDerived() override;

                    void RenderBeforeGUIDerived() override;

                    void RenderAfterGUIDerived() override;

                    void OnKeyDownDerived(SDL_Keycode key) override;

                    void OnKeyUpDerived(SDL_Keycode key) override;

                    void OnMouseDownDerived(Uint8 button) override;

                    void OnMouseUpDerived(Uint8 button, int clickSpeed) override;

                  private:
                    class WorldView
                    {
                      public:
                        void Render();
                    } worldView_;
                };

                void AddScene(std::string_view sceneName, std::shared_ptr<IScene> scene);

                int currentScene_{0};
                std::unordered_map<int, std::shared_ptr<IScene>> scenes_;
            } sceneManager_;

            void PollEvents();

            bool running_{true};
            int ticksLeftMouseButtonDown_{0};
            int ticksRightMouseButtonDown_{0};
        } engine_;
    };
}
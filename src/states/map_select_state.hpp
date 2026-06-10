#pragma once

#include <states/game_state.hpp>
#include <engine/features/ui_widgets.hpp>
#include <raylib.h>
#include <string>
#include <vector>

// Scrollable map picker shown after a datapack is chosen and before gameplay.
// Lists the active pack's custom maps (name, description, preview image from
// "maps/<name>/map.png") plus a final "Auto-Generated" option. Selecting a map
// records the choice in GameData and starts PlayingState; Back returns to the
// datapack picker. Preview textures are owned here and freed on exit.
class MapSelectState : public GameState {
public:
    void OnEnter(Game& game) override;
    void OnExit(Game& game) override;

    void ProcessInput(Game& game, float dt) override;
    void Update(Game& game, float dt) override;
    void Draw(Game& game) override;

private:
    struct MapEntry {
        std::string m_folder;       // subdir name under maps/ (empty for the auto entry)
        std::string m_name;
        std::string m_description;
        Texture2D m_preview = {};
        bool m_hasPreview = false;
        bool m_isAuto = false;      // the synthetic "Auto-Generated" choice
    };

    void RebuildList(Game& game);   // scan maps/, parse meta, load previews
    void UnloadPreviews();          // free every loaded preview texture
    void SelectEntry(Game& game, int index);

    // On-screen rect of card i, accounting for the current scroll offset.
    Rectangle CardRect(Game& game, int index) const;
    float ListTop() const { return kListTop; }
    float ListBottom(Game& game) const;
    float MaxScroll(Game& game) const;

    std::vector<MapEntry> m_entries;
    float m_scroll = 0.0f;
    int m_hovered = -1;
    Button m_backButton;

    // Layout constants (raw virtual coords, matching DatapackSelectState).
    static constexpr float kMargin      = 40.0f;
    static constexpr float kListTop     = 110.0f;
    static constexpr float kFooterH     = 80.0f;
    static constexpr float kCardH       = 120.0f;
    static constexpr float kCardGap     = 12.0f;
    static constexpr float kIconPad     = 16.0f;
    static constexpr float kScrollSpeed = 40.0f;
};

#include <states/map_select_state.hpp>
#include <states/datapack_select_state.hpp>
#include <states/play_state.hpp>
#include <engine/core/text.hpp>
#include <game.hpp>
#include <toml++/toml.hpp>
#include <raylib.h>
#include <algorithm>
#include <memory>
#include <string>

namespace {
    constexpr Color kBackground = {30, 30, 35, 255};
    constexpr Color kSubtle     = {160, 160, 170, 255};
    constexpr Color kAutoTint   = {45, 55, 70, 255};

    constexpr float kThumbW = 160.0f; // preview column width inside a card

    void DrawCenteredText(const char* text, float centerX, float y, int fontSize, Color color) {
        int w = Text::Measure(text, fontSize);
        Text::Draw(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), fontSize, color);
    }

    // Trims text with a trailing ellipsis so it fits within maxWidth pixels.
    std::string TruncateToWidth(const std::string& text, int fontSize, float maxWidth) {
        if (Text::Measure(text.c_str(), fontSize) <= maxWidth) return text;
        std::string out = text;
        while (!out.empty() && Text::Measure((out + "...").c_str(), fontSize) > maxWidth)
            out.pop_back();
        return out + "...";
    }

    // Draw a texture aspect-fitted (letterboxed) inside region, centered.
    void DrawTextureFitted(const Texture2D& tex, Rectangle region) {
        if (tex.id == 0 || tex.width == 0 || tex.height == 0) return;
        float scale = std::min(region.width / tex.width, region.height / tex.height);
        float w = tex.width * scale;
        float h = tex.height * scale;
        Rectangle dst = {region.x + (region.width - w) / 2.0f,
                         region.y + (region.height - h) / 2.0f, w, h};
        Rectangle src = {0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height)};
        DrawTexturePro(tex, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);
    }
}

// --- Lifecycle ---------------------------------------------------------------

void MapSelectState::OnEnter(Game& game) {
    RebuildList(game);

    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float screenH = static_cast<float>(game.GetScreen().GetGameHeight());
    m_backButton.m_label = "BACK";
    m_backButton.m_rect = {screenW / 2.0f - 80.0f, screenH - kFooterH + 18.0f, 160.0f, 44.0f};
}

void MapSelectState::OnExit(Game& /*game*/) {
    UnloadPreviews();
}

// --- Setup -------------------------------------------------------------------

void MapSelectState::RebuildList(Game& game) {
    UnloadPreviews();
    m_entries.clear();
    m_scroll = 0.0f;
    m_hovered = -1;

    FileStore& fs = game.GetFileStore();
    std::string mapsDir = game.GetActiveMapsDir();

    for (const std::string& folder : fs.ListSubfolders(mapsDir)) {
        std::string mapDir = mapsDir + "/" + folder;
        std::string tomlPath = mapDir + "/map.toml";
        if (!fs.Exists(tomlPath)) continue;

        toml::table t = fs.LoadToml(tomlPath);
        if (t.empty()) continue; // missing/corrupt — skip

        MapEntry e;
        e.m_folder = folder;
        e.m_name = t["meta"]["name"].value_or(folder);
        e.m_description = t["meta"]["description"].value_or(std::string{});

        // Preview: bytes come from disk (desktop) or VFS/localStorage (web), so the
        // image pipeline is identical on both platforms.
        std::vector<unsigned char> bytes = fs.LoadBytes(mapDir + "/map.png");
        if (!bytes.empty()) {
            Image img = LoadImageFromMemory(".png", bytes.data(), static_cast<int>(bytes.size()));
            if (img.data != nullptr) {
                e.m_preview = LoadTextureFromImage(img);
                UnloadImage(img);
                e.m_hasPreview = e.m_preview.id != 0;
            }
        }
        m_entries.push_back(std::move(e));
    }

    // The procedural option is always offered, as the final card.
    MapEntry autoEntry;
    autoEntry.m_isAuto = true;
    autoEntry.m_name = "Auto-Generated";
    autoEntry.m_description = "A fresh, procedurally generated map.";
    m_entries.push_back(std::move(autoEntry));
}

void MapSelectState::UnloadPreviews() {
    for (MapEntry& e : m_entries) {
        if (e.m_hasPreview) {
            UnloadTexture(e.m_preview);
            e.m_preview = {};
            e.m_hasPreview = false;
        }
    }
}

// --- Layout helpers ----------------------------------------------------------

float MapSelectState::ListBottom(Game& game) const {
    return static_cast<float>(game.GetScreen().GetGameHeight()) - kFooterH;
}

float MapSelectState::MaxScroll(Game& game) const {
    float contentH = m_entries.size() * (kCardH + kCardGap);
    float bandH = ListBottom(game) - ListTop();
    return std::max(0.0f, contentH - bandH);
}

Rectangle MapSelectState::CardRect(Game& game, int index) const {
    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float cardW = screenW - 2.0f * kMargin;
    float y = ListTop() - m_scroll + index * (kCardH + kCardGap);
    return {kMargin, y, cardW, kCardH};
}

// --- Selection ---------------------------------------------------------------

void MapSelectState::SelectEntry(Game& game, int index) {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return;
    const MapEntry& e = m_entries[index];

    if (e.m_isAuto)
        game.GetGameData().m_selectedMapDir.clear();
    else
        game.GetGameData().m_selectedMapDir = game.GetActiveMapsDir() + "/" + e.m_folder;

    game.ChangeState(std::make_unique<PlayingState>());
}

// --- Input -------------------------------------------------------------------

void MapSelectState::ProcessInput(Game& game, float /*dt*/) {
    Vector2 mouse = game.GetInput().GetMousePosition();
    bool clicked = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);

    float wheel = game.GetInput().GetMouseWheelDelta();
    if (wheel != 0.0f)
        m_scroll = std::clamp(m_scroll - wheel * kScrollSpeed, 0.0f, MaxScroll(game));

    m_backButton.Update(mouse, clicked);
    if (m_backButton.IsClicked() || game.GetInput().IsPressed("Cancel")) {
        game.ChangeState(std::make_unique<DatapackSelectState>(DatapackSelectState::Intent::Play));
        return;
    }

    m_hovered = -1;
    bool inBand = mouse.y >= ListTop() && mouse.y <= ListBottom(game);
    for (int i = 0; i < static_cast<int>(m_entries.size()); i++) {
        Rectangle rect = CardRect(game, i);
        if (inBand && CheckCollisionPointRec(mouse, rect)) {
            m_hovered = i;
            if (clicked) {
                SelectEntry(game, i);
                return;
            }
        }
    }
}

void MapSelectState::Update(Game& /*game*/, float /*dt*/) {}

// --- Draw --------------------------------------------------------------------

void MapSelectState::Draw(Game& game) {
    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float screenH = static_cast<float>(game.GetScreen().GetGameHeight());
    float listBottom = ListBottom(game);

    ClearBackground(kBackground);

    for (int i = 0; i < static_cast<int>(m_entries.size()); i++) {
        const MapEntry& entry = m_entries[i];
        Rectangle card = CardRect(game, i);

        // Cull cards entirely outside the visible band.
        if (card.y + card.height < ListTop() || card.y > listBottom) continue;

        bool hovered = (i == m_hovered);
        DrawRectangleRec(card, hovered ? kDefaultStyle.m_bgHovered : kDefaultStyle.m_bgNormal);
        DrawRectangleLinesEx(card, hovered ? kDefaultStyle.m_borderWidthActive : kDefaultStyle.m_borderWidth,
                             hovered ? kDefaultStyle.m_borderSel : kDefaultStyle.m_border);

        // Thumbnail column.
        Rectangle thumb = {card.x + kIconPad, card.y + kIconPad, kThumbW, kCardH - 2.0f * kIconPad};
        if (entry.m_isAuto) {
            DrawRectangleRec(thumb, kAutoTint);
            DrawRectangleLinesEx(thumb, 1.0f, kDefaultStyle.m_border);
            DrawCenteredText("AUTO", thumb.x + thumb.width / 2.0f,
                             thumb.y + thumb.height / 2.0f - 12.0f, 24, kDefaultStyle.m_accent);
        } else if (entry.m_hasPreview) {
            DrawRectangleRec(thumb, {15, 15, 18, 255});
            DrawTextureFitted(entry.m_preview, thumb);
            DrawRectangleLinesEx(thumb, 1.0f, kDefaultStyle.m_border);
        } else {
            DrawRectangleRec(thumb, {20, 20, 25, 255});
            DrawRectangleLinesEx(thumb, 1.0f, kDefaultStyle.m_border);
            DrawCenteredText("no preview", thumb.x + thumb.width / 2.0f,
                             thumb.y + thumb.height / 2.0f - 8.0f, 16, kSubtle);
        }

        // Text column.
        float textX = thumb.x + thumb.width + 20.0f;
        float textRight = card.x + card.width - 20.0f;
        float textW = textRight - textX;

        Color nameColor = entry.m_isAuto ? kDefaultStyle.m_accent : kDefaultStyle.m_text;
        Text::Draw(entry.m_name.c_str(), static_cast<int>(textX), static_cast<int>(card.y + 24.0f),
                   26, nameColor);

        std::string desc = TruncateToWidth(entry.m_description, 16, textW);
        Text::Draw(desc.c_str(), static_cast<int>(textX), static_cast<int>(card.y + 64.0f), 16, kSubtle);
    }

    // Scrollbar (only when there is something to scroll).
    float maxScroll = MaxScroll(game);
    if (maxScroll > 0.0f) {
        float bandH = listBottom - ListTop();
        float contentH = bandH + maxScroll;
        float trackX = screenW - kMargin + 8.0f;
        float thumbH = bandH * (bandH / contentH);
        float thumbY = ListTop() + (m_scroll / maxScroll) * (bandH - thumbH);
        DrawRectangle(static_cast<int>(trackX), static_cast<int>(ListTop()), 6, static_cast<int>(bandH), {20, 20, 25, 255});
        DrawRectangle(static_cast<int>(trackX), static_cast<int>(thumbY), 6, static_cast<int>(thumbH), kDefaultStyle.m_border);
    }

    // Header mask + title (covers any card scrolled up into this band).
    DrawRectangle(0, 0, static_cast<int>(screenW), static_cast<int>(ListTop()), kBackground);
    DrawCenteredText("SELECT MAP", screenW / 2.0f, 40.0f, 40, RAYWHITE);

    // Footer mask + back button.
    DrawRectangle(0, static_cast<int>(listBottom), static_cast<int>(screenW),
                  static_cast<int>(screenH - listBottom), kBackground);
    m_backButton.Draw();
    m_backButton.DrawLabel(24, RAYWHITE);
}

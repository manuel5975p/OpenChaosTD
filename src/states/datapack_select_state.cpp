#include <states/datapack_select_state.hpp>
#include <states/menu_state.hpp>
#include <states/play_state.hpp>
#include <states/particle_editor_state.hpp>
#include <states/map_editor_state.hpp>
#include <datapack/datapack.hpp>
#include <engine/core/text.hpp>
#include <game.hpp>
#include <raylib.h>
#include <algorithm>
#include <memory>
#include <string>

namespace {
    constexpr Color kBackground = {30, 30, 35, 255};
    constexpr Color kSubtle     = {160, 160, 170, 255};

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
}

void DatapackSelectState::OnEnter(Game& game) {
    // Rescan so packs dropped in since launch show up, then load their icons.
    game.GetDatapackRegistry().Scan(game.GetFileStore());
    game.GetDatapackRegistry().LoadIcons();

    m_scroll = 0.0f;
    m_hovered = -1;

    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float screenH = static_cast<float>(game.GetScreen().GetGameHeight());
    m_backButton.m_label = "BACK";
    m_backButton.m_rect = { screenW / 2.0f - 80.0f, screenH - kFooterH + 18.0f, 160.0f, 44.0f };
}

void DatapackSelectState::OnExit(Game& game) {
    // Selection-screen thumbnails are only needed while this screen is up.
    game.GetDatapackRegistry().UnloadIcons();
}

float DatapackSelectState::ListBottom(Game& game) const {
    return static_cast<float>(game.GetScreen().GetGameHeight()) - kFooterH;
}

float DatapackSelectState::MaxScroll(Game& game) const {
    int count = static_cast<int>(game.GetDatapackRegistry().Packs().size());
    float contentH = count * (kCardH + kCardGap);
    float bandH = ListBottom(game) - ListTop();
    return std::max(0.0f, contentH - bandH);
}

Rectangle DatapackSelectState::CardRect(Game& game, int index) const {
    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float cardW = screenW - 2.0f * kMargin;
    float y = ListTop() - m_scroll + index * (kCardH + kCardGap);
    return { kMargin, y, cardW, kCardH };
}

void DatapackSelectState::SelectPack(Game& game, int index) {
    const auto& packs = game.GetDatapackRegistry().Packs();
    if (index < 0 || index >= static_cast<int>(packs.size())) return;

    game.ActivateDatapack(packs[index]);

    if (m_intent == Intent::EditParticles)
        game.ChangeState(std::make_unique<ParticleEditorState>());
    else if (m_intent == Intent::EditMap)
        game.ChangeState(std::make_unique<MapEditorState>());
    else if (m_intent == Intent::Continue)
        game.ChangeState(std::make_unique<PlayingState>(true));
    else
        game.ChangeState(std::make_unique<PlayingState>());
}

void DatapackSelectState::ProcessInput(Game& game, float /*dt*/) {
    Vector2 mouse = game.GetInput().GetMousePosition();
    bool clicked = game.GetInput().IsMousePressed(MOUSE_LEFT_BUTTON);

    // Scroll wheel pans the list, clamped to the content extent.
    float wheel = game.GetInput().GetMouseWheelDelta();
    if (wheel != 0.0f)
        m_scroll = std::clamp(m_scroll - wheel * kScrollSpeed, 0.0f, MaxScroll(game));

    // Back button / cancel returns to the menu.
    m_backButton.Update(mouse, clicked);
    if (m_backButton.IsClicked() || game.GetInput().IsPressed("Cancel")) {
        game.ChangeState(std::make_unique<MenuState>());
        return;
    }

    // Card hover/select — only within the visible band so clicks on the masked
    // overflow above/below the list are ignored.
    m_hovered = -1;
    const auto& packs = game.GetDatapackRegistry().Packs();
    bool inBand = mouse.y >= ListTop() && mouse.y <= ListBottom(game);
    for (int i = 0; i < static_cast<int>(packs.size()); i++) {
        Rectangle rect = CardRect(game, i);
        if (inBand && CheckCollisionPointRec(mouse, rect)) {
            m_hovered = i;
            if (clicked) {
                SelectPack(game, i);
                return;
            }
        }
    }
}

void DatapackSelectState::Update(Game& /*game*/, float /*dt*/) {}

void DatapackSelectState::Draw(Game& game) {
    float screenW = static_cast<float>(game.GetScreen().GetGameWidth());
    float screenH = static_cast<float>(game.GetScreen().GetGameHeight());
    float listBottom = ListBottom(game);

    ClearBackground(kBackground);

    const auto& packs = game.GetDatapackRegistry().Packs();

    if (packs.empty()) {
        DrawCenteredText("No datapacks found", screenW / 2.0f, screenH / 2.0f - 12.0f, 28, kSubtle);
    }

    // Cards. Overflow past the band is hidden by the header/footer masks below.
    for (int i = 0; i < static_cast<int>(packs.size()); i++) {
        const Datapack& pack = packs[i];
        Rectangle card = CardRect(game, i);

        // Cheap cull: skip cards entirely outside the band.
        if (card.y + card.height < ListTop() || card.y > listBottom) continue;

        bool hovered = (i == m_hovered);
        DrawRectangleRec(card, hovered ? kDefaultStyle.m_bgHovered : kDefaultStyle.m_bgNormal);
        DrawRectangleLinesEx(card, hovered ? kDefaultStyle.m_borderWidthActive : kDefaultStyle.m_borderWidth,
                             hovered ? kDefaultStyle.m_borderSel : kDefaultStyle.m_border);

        // Icon (or a placeholder when the pack ships none).
        float iconSize = kCardH - 2.0f * kIconPad;
        Rectangle iconRect = { card.x + kIconPad, card.y + kIconPad, iconSize, iconSize };
        if (pack.m_icon.id != 0) {
            Rectangle src = { 0.0f, 0.0f,
                              static_cast<float>(pack.m_icon.width),
                              static_cast<float>(pack.m_icon.height) };
            DrawTexturePro(pack.m_icon, src, iconRect, { 0.0f, 0.0f }, 0.0f, WHITE);
        } else {
            DrawRectangleRec(iconRect, {20, 20, 25, 255});
            DrawRectangleLinesEx(iconRect, 1.0f, kDefaultStyle.m_border);
        }

        // Text column.
        float textX = iconRect.x + iconSize + 20.0f;
        float textRight = card.x + card.width - 20.0f;
        float textW = textRight - textX;

        Text::Draw(pack.m_name.c_str(), static_cast<int>(textX), static_cast<int>(card.y + 16.0f),
                   26, kDefaultStyle.m_text);

        std::string meta = "by " + pack.m_author + "    v" + pack.m_version;
        Text::Draw(meta.c_str(), static_cast<int>(textX), static_cast<int>(card.y + 50.0f), 16, kDefaultStyle.m_accent);

        std::string desc = TruncateToWidth(pack.m_description, 16, textW);
        Text::Draw(desc.c_str(), static_cast<int>(textX), static_cast<int>(card.y + 78.0f), 16, kSubtle);
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

    // Header mask + title (drawn over any card that scrolled up into this band).
    DrawRectangle(0, 0, static_cast<int>(screenW), static_cast<int>(ListTop()), kBackground);
    DrawCenteredText("SELECT DATAPACK", screenW / 2.0f, 40.0f, 40, RAYWHITE);

    // Footer mask + back button.
    DrawRectangle(0, static_cast<int>(listBottom), static_cast<int>(screenW),
                  static_cast<int>(screenH - listBottom), kBackground);
    m_backButton.Draw();
    m_backButton.DrawLabel(24, RAYWHITE);
}

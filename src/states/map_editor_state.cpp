#include <states/map_editor_state.hpp>
#include <states/menu_state.hpp>
#include <engine/core/text.hpp>
#include <world/tile.hpp>
#include <game.hpp>
#include <raylib.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <string>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

namespace {
    constexpr Color kAccentColor = {255, 180, 0, 255};
    constexpr Color kFailColor   = {255, 120, 60, 255};
    constexpr float kScrollSpeed = 30.0f;

    // Stat keys a Buff tile can apply, parallel to the three buff brush buttons.
    const char* kBuffStats[3] = {"range", "damage", "shotsPerMinute"};

    void DrawCenteredText(const char* text, float centerX, float y, int fontSize, Color color) {
        int w = Text::Measure(text, fontSize);
        Text::Draw(text, static_cast<int>(centerX - w / 2.0f), static_cast<int>(y), fontSize, color);
    }

    void DrawLabelInRow(const char* text, float x, float rowY, float rowH, int fontSize, Color color) {
        Text::Draw(text, static_cast<int>(x), static_cast<int>(rowY + (rowH - fontSize) / 2.0f), fontSize, color);
    }

    // Translucent tint for the active brush ghost / hover highlight. Index = Brush value.
    Color BrushTint(int brush) {
        switch (brush) {
            case 1: return {110, 110, 120, 130}; // Rock
            case 2: return {255, 200, 50, 150};  // Core
            case 3: return {220, 70, 90, 150};    // Nest
            case 4: return {100, 149, 237, 150}; // Buff
            default: return {90, 200, 90, 130};   // Grass
        }
    }
}

// --- Lifecycle ---------------------------------------------------------------

void MapEditorState::OnEnter(Game& game) {
    const char* brushNames[5] = {"GRASS", "ROCK", "CORE", "NEST", "BUFF"};
    for (int i = 0; i < 5; i++)
        m_brushButtons[i].m_label = brushNames[i];

    const char* statNames[3] = {"RANGE", "DAMAGE", "SPEED"};
    for (int i = 0; i < 3; i++)
        m_buffStatButtons[i].m_label = statNames[i];
    m_buffMul.m_label = "Multiply";

    m_newMapBtn.m_label      = "NEW MAP";
    m_catalogBackBtn.m_label = "BACK";
    m_modalCreateBtn.m_label = "CREATE";
    m_modalCancelBtn.m_label = "CANCEL";
    m_validateBtn.m_label    = "VALIDATE";
    m_saveBtn.m_label        = "SAVE";
    m_editBackBtn.m_label    = "BACK";

    m_modalName.m_maxLength = 32;
    m_modalDesc.m_maxLength = 120;
    m_modalCols.m_min = kMinDim; m_modalCols.m_max = kMaxDim; m_modalCols.m_step = 1.0f;
    m_modalRows.m_min = kMinDim; m_modalRows.m_max = kMaxDim; m_modalRows.m_step = 1.0f;
    m_modalCols.m_value = kDefaultCols;
    m_modalRows.m_value = kDefaultRows;

    SyncBuffControls();
    Layout(game);
    m_mode = Mode::Catalog;
    RebuildCatalog(game);
}

void MapEditorState::OnExit(Game& /*game*/) {}

// --- Setup / layout ----------------------------------------------------------

void MapEditorState::Layout(Game& game) {
    float gw = static_cast<float>(game.GetScreen().GetGameWidth());
    float gh = static_cast<float>(game.GetScreen().GetGameHeight());
    float footerY = gh - kFooterH;

    // Catalog list band + footer actions.
    m_catalogRect = {kMargin, kTopY, gw - 2.0f * kMargin, footerY - kTopY};
    m_newMapBtn.m_rect      = {kMargin, footerY + 18.0f, 180.0f, 44.0f};
    m_catalogBackBtn.m_rect = {gw - kMargin - 160.0f, footerY + 18.0f, 160.0f, 44.0f};

    // New-map modal, centered.
    float mw = 480.0f, mh = 330.0f;
    m_modalRect = {(gw - mw) / 2.0f, (gh - mh) / 2.0f, mw, mh};
    float mx = m_modalRect.x + 30.0f;
    float my = m_modalRect.y + 64.0f;
    m_modalName.m_rect = {mx, my, mw - 60.0f, 40.0f};
    m_modalDesc.m_rect = {mx, my + 64.0f, mw - 60.0f, 40.0f};
    m_modalCols.m_rect = {mx + 90.0f, my + 138.0f, mw - 180.0f, 24.0f};
    m_modalRows.m_rect = {mx + 90.0f, my + 174.0f, mw - 180.0f, 24.0f};
    m_modalCreateBtn.m_rect = {mx, m_modalRect.y + mh - 58.0f, 170.0f, 40.0f};
    m_modalCancelBtn.m_rect = {m_modalRect.x + mw - 30.0f - 170.0f, m_modalRect.y + mh - 58.0f, 170.0f, 40.0f};

    // Edit palette (left column).
    float py = kTopY + 30.0f;
    for (int i = 0; i < 5; i++) {
        m_brushButtons[i].m_rect = {kPaletteX, py, kPaletteW, kBrushBtnH};
        py += kBrushBtnH + kRowGap;
    }
    py += 26.0f; // gap before the buff sub-controls
    float sw = (kPaletteW - 2.0f * 6.0f) / 3.0f;
    for (int i = 0; i < 3; i++)
        m_buffStatButtons[i].m_rect = {kPaletteX + i * (sw + 6.0f), py, sw, 30.0f};
    py += 42.0f;
    m_buffValue.m_rect = {kPaletteX, py, kPaletteW, 24.0f};
    py += 38.0f;
    m_buffMul.m_rect = {kPaletteX, py, 26.0f, 26.0f};

    // Edit canvas + bottom action bar.
    m_canvasRect = {kCanvasX, kTopY, gw - kMargin - kCanvasX, footerY - kTopY};
    float by = footerY + 18.0f;
    m_validateBtn.m_rect = {kCanvasX, by, 160.0f, 44.0f};
    m_saveBtn.m_rect     = {kCanvasX + 176.0f, by, 160.0f, 44.0f};
    m_editBackBtn.m_rect = {gw - kMargin - 160.0f, by, 160.0f, 44.0f};
}

void MapEditorState::RebuildCatalog(Game& game) {
    m_mapFolders = game.GetFileStore().ListSubfolders(MapsDir(game));
    m_openButtons.assign(m_mapFolders.size(), Button{});
    m_deleteButtons.assign(m_mapFolders.size(), Button{});
    for (size_t i = 0; i < m_mapFolders.size(); i++) {
        m_openButtons[i].m_label = "OPEN";
        m_deleteButtons[i].m_label = "DELETE";
    }
    m_catalogScroll = 0.0f;
}

void MapEditorState::SyncBuffControls() {
    if (m_buffStatIndex == 0) {
        // range: a flat bonus added to the tower's range.
        m_buffValue.m_min = 0.0f; m_buffValue.m_max = 200.0f; m_buffValue.m_step = 5.0f;
        m_buffValue.m_value = 30.0f;
        m_buffMul.m_value = false;
    } else {
        // damage / shotsPerMinute: a multiplier on the tower's stat.
        m_buffValue.m_min = 0.0f; m_buffValue.m_max = 3.0f; m_buffValue.m_step = 0.1f;
        m_buffValue.m_value = 1.5f;
        m_buffMul.m_value = true;
    }
}

// --- Helpers -----------------------------------------------------------------

void MapEditorState::SetStatus(const std::string& msg, bool ok) {
    m_status = msg;
    m_statusOk = ok;
    m_statusTimer = 3.0f;
}

void MapEditorState::SanitizeName(std::string& name) {
    name.erase(std::remove_if(name.begin(), name.end(), [](unsigned char c) {
        return !(std::isalnum(c) || c == '_' || c == '-');
    }), name.end());
}

std::string MapEditorState::MapsDir(Game& game) const {
    return game.GetActiveMapsDir();
}

std::string MapEditorState::MapDir(Game& game, const std::string& folder) const {
    return MapsDir(game) + "/" + folder;
}

// --- Catalog actions ---------------------------------------------------------

void MapEditorState::OpenMap(Game& game, int index) {
    if (index < 0 || index >= static_cast<int>(m_mapFolders.size())) return;
    const std::string& folder = m_mapFolders[index];

    if (!MapSerialization::Load(game.GetFileStore(), MapDir(game, folder), m_map, m_meta)) {
        SetStatus("Could not load '" + folder + "'", false);
        return;
    }
    m_openFolder = folder;
    m_mode = Mode::Edit;
    m_brush = Brush::Grass;
    m_lastValidateOk = false;
    m_hoverX = m_hoverY = -1;
    m_render.CenterCamera(m_map, game.GetScreen());
}

void MapEditorState::DeleteMap(Game& game, int index) {
    if (index < 0 || index >= static_cast<int>(m_mapFolders.size())) return;
    const std::string folder = m_mapFolders[index];
    game.GetFileStore().DeleteFolder(MapDir(game, folder));
    RebuildCatalog(game);
    SetStatus("Deleted '" + folder + "'", true);
}

void MapEditorState::ConfirmNewMap(Game& game) {
    std::string name = m_modalName.m_text;
    SanitizeName(name);
    if (name.empty()) {
        SetStatus("Enter a map name", false);
        return;
    }
    if (game.GetFileStore().Exists(MapDir(game, name) + "/map.toml")) {
        SetStatus("A map named '" + name + "' already exists", false);
        return;
    }

    int cols = static_cast<int>(std::lround(m_modalCols.m_value));
    int rows = static_cast<int>(std::lround(m_modalRows.m_value));
    m_map.Create(cols, rows); // all-grass, walkable/buildable by default

    m_meta.m_name = name;
    m_meta.m_description = m_modalDesc.m_text;
    m_openFolder = name;

    m_modalOpen = false;
    m_mode = Mode::Edit;
    m_brush = Brush::Grass;
    m_lastValidateOk = false;
    m_hoverX = m_hoverY = -1;
    m_render.CenterCamera(m_map, game.GetScreen());
    SetStatus("New map - paint a core, nests and a path, then save", true);
}

// --- Edit actions ------------------------------------------------------------

void MapEditorState::PaintAt(int tx, int ty) {
    Tile& tile = m_map.Get(tx, ty);
    switch (m_brush) {
        case Brush::Grass:
            tile.m_type = TileType::Grass;
            tile.m_walkable = true;
            tile.m_buildable = true;
            tile.m_modifier = {};
            break;
        case Brush::Rock:
            tile.m_type = TileType::Rock;
            tile.m_walkable = false;
            tile.m_buildable = false;
            tile.m_modifier = {};
            break;
        case Brush::Core: {
            // Enforce a single core: clear the previous core tile back to grass first.
            std::pair<int, int> core = m_map.GetCore();
            if (m_map.GetGrid().InBounds(core.first, core.second)) {
                Tile& old = m_map.Get(core.first, core.second);
                if (old.m_type == TileType::Core) {
                    old.m_type = TileType::Grass;
                    old.m_walkable = true;
                    old.m_buildable = true;
                }
            }
            m_map.SetCore(tx, ty);
            break;
        }
        case Brush::Nest:
            m_map.AddNest(tx, ty); // idempotent — AddNest dedups
            break;
        case Brush::Buff:
            m_map.SetBuff(tx, ty, kBuffStats[m_buffStatIndex], m_buffValue.m_value, m_buffMul.m_value);
            break;
    }
}

bool MapEditorState::Validate() {
    // Re-derive core/nests from the painted grid so stale geometry can't slip through.
    m_map.RebuildGeometryFromGrid();

    if (!m_map.GetGrid().InBounds(m_map.GetCore().first, m_map.GetCore().second)) {
        SetStatus("Validation failed: place a Core", false);
        m_lastValidateOk = false;
        return false;
    }
    if (m_map.GetNests().empty()) {
        SetStatus("Validation failed: place at least one Nest", false);
        m_lastValidateOk = false;
        return false;
    }
    if (!m_map.ValidatePathMesh()) {
        SetStatus("Validation failed: a Nest cannot reach the Core", false);
        m_lastValidateOk = false;
        return false;
    }

    SetStatus("Valid: all nests reach the core", true);
    m_lastValidateOk = true;
    return true;
}

void MapEditorState::Save(Game& game) {
    if (!Validate())
        return; // Validate already set a descriptive blocking status

    std::string dir = MapDir(game, m_openFolder);
    if (!MapSerialization::Save(game.GetFileStore(), dir, m_map, m_meta)) {
        SetStatus("Save failed - see log", false);
        return;
    }
    ExportPng(game, dir);
    SetStatus("Saved '" + m_meta.m_name + "'", true);
}

void MapEditorState::ExportPng(Game& game, const std::string& mapDir) {
    int w = m_map.GetCols() * m_map.GetTileSize();
    int h = m_map.GetRows() * m_map.GetTileSize();

    // Render the grid at 1:1 pixel scale, no camera, into an offscreen texture.
    RenderTexture2D target = LoadRenderTexture(w, h);
    BeginTextureMode(target);
    ClearBackground(BLACK);
    m_render.DrawMap(m_map, game.GetResources());
    EndTextureMode();

    Image img = LoadImageFromTexture(target.texture);
    ImageFlipVertical(&img); // render textures are stored bottom-up

    std::string pngPath = mapDir + "/map.png";
    ExportImage(img, game.GetFileStore().FullPath(pngPath).c_str());
    UnloadImage(img);
    UnloadRenderTexture(target);

#if defined(PLATFORM_WEB)
    // ExportImage wrote to MEMFS; copy it into localStorage (base64) so the preview
    // survives a reload, matching how the rest of FileStore persists on web.
    EM_ASM({
        var path = UTF8ToString($0);
        try {
            var data = FS.readFile(path);
            var bin = '';
            for (var i = 0; i < data.length; i++) bin += String.fromCharCode(data[i]);
            localStorage.setItem(path, btoa(bin));
        } catch (e) { console.error('map.png persist failed', e); }
    }, pngPath.c_str());
#endif
}

// --- Input -------------------------------------------------------------------

void MapEditorState::ProcessInput(Game& game, float dt) {
    if (m_mode == Mode::Catalog) {
        if (m_modalOpen) ProcessModalInput(game);
        else             ProcessCatalogInput(game);
    } else {
        ProcessEditInput(game, dt);
    }
}

void MapEditorState::ProcessCatalogInput(Game& game) {
    Input& input = game.GetInput();
    Vector2 mouse = input.GetMousePosition();
    bool clicked = input.IsMousePressed(MOUSE_LEFT_BUTTON);

    float contentH = m_mapFolders.size() * (kRowH + kRowGap);
    float maxScroll = std::max(0.0f, contentH - m_catalogRect.height);
    float wheel = input.GetMouseWheelDelta();
    if (wheel != 0.0f && CheckCollisionPointRec(mouse, m_catalogRect))
        m_catalogScroll = std::clamp(m_catalogScroll - wheel * kScrollSpeed, 0.0f, maxScroll);

    m_newMapBtn.Update(mouse, clicked);
    m_catalogBackBtn.Update(mouse, clicked);
    if (m_catalogBackBtn.IsClicked() || input.IsPressed("Cancel")) {
        game.ChangeState(std::make_unique<MenuState>());
        return;
    }
    if (m_newMapBtn.IsClicked()) {
        m_modalOpen = true;
        m_modalName.m_text.clear();
        m_modalDesc.m_text.clear();
        m_modalCols.m_value = kDefaultCols;
        m_modalRows.m_value = kDefaultRows;
        return;
    }

    // Map rows: position each row's buttons for the current scroll, then hit-test
    // only within the visible band so masked overflow stays inert.
    float rowRight = m_catalogRect.x + m_catalogRect.width;
    bool inBand = CheckCollisionPointRec(mouse, m_catalogRect);
    for (size_t i = 0; i < m_mapFolders.size(); i++) {
        float rowY = m_catalogRect.y - m_catalogScroll + i * (kRowH + kRowGap);
        m_openButtons[i].m_rect   = {rowRight - 204.0f, rowY + 4.0f, 90.0f, kRowH - 8.0f};
        m_deleteButtons[i].m_rect = {rowRight - 104.0f, rowY + 4.0f, 90.0f, kRowH - 8.0f};
        if (!inBand) continue;

        m_openButtons[i].Update(mouse, clicked);
        m_deleteButtons[i].Update(mouse, clicked);
        if (m_openButtons[i].IsClicked()) {
            OpenMap(game, static_cast<int>(i));
            return;
        }
        if (m_deleteButtons[i].IsClicked()) {
            DeleteMap(game, static_cast<int>(i));
            return;
        }
    }
}

void MapEditorState::ProcessModalInput(Game& game) {
    Input& input = game.GetInput();
    Vector2 mouse = input.GetMousePosition();
    bool pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);
    bool down = input.IsMouseDown(MOUSE_LEFT_BUTTON);

    m_modalName.Update(mouse, pressed);
    SanitizeName(m_modalName.m_text);
    m_modalDesc.Update(mouse, pressed);
    m_modalCols.Update(mouse, down);
    m_modalRows.Update(mouse, down);
    m_modalCreateBtn.Update(mouse, pressed);
    m_modalCancelBtn.Update(mouse, pressed);

    bool typing = m_modalName.IsFocused() || m_modalDesc.IsFocused();
    if (m_modalCancelBtn.IsClicked() || (input.IsPressed("Cancel") && !typing)) {
        m_modalOpen = false;
        return;
    }
    if (m_modalCreateBtn.IsClicked())
        ConfirmNewMap(game);
}

void MapEditorState::ProcessEditInput(Game& game, float dt) {
    Input& input = game.GetInput();
    Vector2 mouse = input.GetMousePosition();
    bool pressed = input.IsMousePressed(MOUSE_LEFT_BUTTON);
    bool down = input.IsMouseDown(MOUSE_LEFT_BUTTON);

    // Brush palette.
    for (int i = 0; i < 5; i++) {
        m_brushButtons[i].Update(mouse, pressed);
        if (m_brushButtons[i].IsClicked())
            m_brush = static_cast<Brush>(i);
    }
    if (m_brush == Brush::Buff) {
        for (int i = 0; i < 3; i++) {
            m_buffStatButtons[i].Update(mouse, pressed);
            if (m_buffStatButtons[i].IsClicked()) {
                m_buffStatIndex = i;
                SyncBuffControls();
            }
        }
        m_buffValue.Update(mouse, down);
        m_buffMul.Update(mouse, pressed);
    }

    // Bottom action bar.
    m_validateBtn.Update(mouse, pressed);
    m_saveBtn.Update(mouse, pressed);
    m_editBackBtn.Update(mouse, pressed);
    if (m_editBackBtn.IsClicked() || input.IsPressed("Cancel")) {
        m_mode = Mode::Catalog;
        RebuildCatalog(game);
        return;
    }
    if (m_validateBtn.IsClicked())
        Validate();
    if (m_saveBtn.IsClicked())
        Save(game);

    // Block grid interaction when the cursor is over the palette/bottom-bar UI.
    float gw = static_cast<float>(game.GetScreen().GetGameWidth());
    float gh = static_cast<float>(game.GetScreen().GetGameHeight());
    Rectangle paletteBand = {0.0f, kTopY, kCanvasX - 10.0f, gh - kFooterH - kTopY};
    Rectangle footerBand  = {0.0f, gh - kFooterH, gw, kFooterH};
    if (CheckCollisionPointRec(mouse, paletteBand) || CheckCollisionPointRec(mouse, footerBand))
        input.ConsumeMouseInput();

    // Canvas: hover ghost, paint on drag, camera pan/zoom.
    m_hoverX = m_hoverY = -1;
    bool overCanvas = CheckCollisionPointRec(mouse, m_canvasRect) && !input.IsMouseInputConsumed();
    if (overCanvas) {
        Vector2 world = input.GetWorldMousePosition(m_render.GetCamera());
        int tx, ty;
        if (m_map.WorldToTile(world, tx, ty)) {
            m_hoverX = tx;
            m_hoverY = ty;
            if (down) {
                PaintAt(tx, ty);
                m_lastValidateOk = false;
            }
        }
        m_render.ControlCamera(dt, input); // right-drag pan + wheel zoom
    }
}

void MapEditorState::Update(Game& /*game*/, float dt) {
    if (m_statusTimer > 0.0f)
        m_statusTimer -= dt;
}

// --- Draw --------------------------------------------------------------------

void MapEditorState::Draw(Game& game) {
    float gw = static_cast<float>(game.GetScreen().GetGameWidth());
    float gh = static_cast<float>(game.GetScreen().GetGameHeight());

    ClearBackground(DARKGRAY);
    DrawCenteredText("MAP EDITOR", gw / 2.0f, 40.0f, 40, RAYWHITE);

    if (m_mode == Mode::Catalog) {
        DrawCatalog(game);
        if (m_modalOpen)
            DrawNewMapModal(game);
    } else {
        DrawPalette(game);
        DrawEditCanvas(game);
        DrawBottomBar(game);
    }

    if (m_statusTimer > 0.0f)
        DrawCenteredText(m_status.c_str(), gw / 2.0f, gh - kFooterH - 26.0f, 18,
                         m_statusOk ? GREEN : kFailColor);
}

void MapEditorState::DrawCatalog(Game& game) {
    if (m_mapFolders.empty()) {
        DrawCenteredText("No maps in this pack - click NEW MAP to create one",
                         m_catalogRect.x + m_catalogRect.width / 2.0f,
                         m_catalogRect.y + m_catalogRect.height / 2.0f - 9.0f, 20, LIGHTGRAY);
    }

    float rowRight = m_catalogRect.x + m_catalogRect.width;
    game.GetScreen().BeginScissor(m_catalogRect);
    for (size_t i = 0; i < m_mapFolders.size(); i++) {
        float rowY = m_catalogRect.y - m_catalogScroll + i * (kRowH + kRowGap);
        // Position the row's buttons here too, so drawing never depends on input order.
        m_openButtons[i].m_rect   = {rowRight - 204.0f, rowY + 4.0f, 90.0f, kRowH - 8.0f};
        m_deleteButtons[i].m_rect = {rowRight - 104.0f, rowY + 4.0f, 90.0f, kRowH - 8.0f};
        if (rowY + kRowH < m_catalogRect.y || rowY > m_catalogRect.y + m_catalogRect.height)
            continue;

        Rectangle row = {m_catalogRect.x, rowY, m_catalogRect.width, kRowH};
        DrawRectangleRec(row, kDefaultStyle.m_bgNormal);
        DrawRectangleLinesEx(row, kDefaultStyle.m_borderWidth, kDefaultStyle.m_border);
        DrawLabelInRow(m_mapFolders[i].c_str(), row.x + 16.0f, row.y, row.height, 22, RAYWHITE);

        m_openButtons[i].Draw();
        m_openButtons[i].DrawLabel(16, RAYWHITE);
        m_deleteButtons[i].Draw(false, kDefaultStyle);
        m_deleteButtons[i].DrawLabel(16, kFailColor);
    }
    game.GetScreen().EndScissor();

    m_newMapBtn.Draw();
    m_newMapBtn.DrawLabel(20, RAYWHITE);
    m_catalogBackBtn.Draw();
    m_catalogBackBtn.DrawLabel(20, RAYWHITE);
}

void MapEditorState::DrawNewMapModal(Game& /*game*/) {
    // Dim the catalog behind the modal.
    DrawRectangleRec(m_modalRect, {25, 25, 30, 255});
    DrawRectangleLinesEx(m_modalRect, 2.0f, kAccentColor);

    DrawCenteredText("NEW MAP", m_modalRect.x + m_modalRect.width / 2.0f,
                     m_modalRect.y + 18.0f, 26, RAYWHITE);

    Text::Draw("Name", static_cast<int>(m_modalName.m_rect.x),
               static_cast<int>(m_modalName.m_rect.y - 18.0f), 14, LIGHTGRAY);
    m_modalName.Draw();
    Text::Draw("Description", static_cast<int>(m_modalDesc.m_rect.x),
               static_cast<int>(m_modalDesc.m_rect.y - 18.0f), 14, LIGHTGRAY);
    m_modalDesc.Draw();

    DrawLabelInRow(TextFormat("Cols  %d", static_cast<int>(std::lround(m_modalCols.m_value))),
                   m_modalRect.x + 30.0f, m_modalCols.m_rect.y, m_modalCols.m_rect.height, 16, RAYWHITE);
    m_modalCols.Draw();
    DrawLabelInRow(TextFormat("Rows  %d", static_cast<int>(std::lround(m_modalRows.m_value))),
                   m_modalRect.x + 30.0f, m_modalRows.m_rect.y, m_modalRows.m_rect.height, 16, RAYWHITE);
    m_modalRows.Draw();

    bool canCreate = !m_modalName.m_text.empty();
    m_modalCreateBtn.Draw(false, canCreate ? kDefaultStyle : kDisabledStyle);
    m_modalCreateBtn.DrawLabel(18, canCreate ? RAYWHITE : Color{120, 120, 120, 255});
    m_modalCancelBtn.Draw();
    m_modalCancelBtn.DrawLabel(18, RAYWHITE);
}

void MapEditorState::DrawPalette(Game& /*game*/) {
    Text::Draw("BRUSH", static_cast<int>(kPaletteX), static_cast<int>(kTopY), 24, kAccentColor);

    for (int i = 0; i < 5; i++) {
        m_brushButtons[i].Draw(m_brush == static_cast<Brush>(i));
        m_brushButtons[i].DrawLabel(18, RAYWHITE);
    }

    if (m_brush != Brush::Buff)
        return;

    float headerY = m_buffStatButtons[0].m_rect.y - 24.0f;
    Text::Draw("BUFF", static_cast<int>(kPaletteX), static_cast<int>(headerY), 18, kAccentColor);
    for (int i = 0; i < 3; i++) {
        m_buffStatButtons[i].Draw(m_buffStatIndex == i);
        m_buffStatButtons[i].DrawLabel(12, RAYWHITE);
    }
    m_buffValue.Draw();
    DrawLabelInRow(TextFormat("%.1f", m_buffValue.m_value),
                   m_buffValue.m_rect.x, m_buffValue.m_rect.y - 22.0f, 20.0f, 14, RAYWHITE);
    m_buffMul.Draw();
    DrawLabelInRow(m_buffMul.m_value ? "Multiply" : "Add",
                   m_buffMul.m_rect.x + 34.0f, m_buffMul.m_rect.y, m_buffMul.m_rect.height, 16, RAYWHITE);
}

void MapEditorState::DrawEditCanvas(Game& game) {
    DrawRectangleRec(m_canvasRect, {18, 18, 22, 255});

    game.GetScreen().BeginScissor(m_canvasRect);
    BeginMode2D(m_render.GetCamera());

    m_render.DrawMap(m_map, game.GetResources());

    // Grid lines over the whole map.
    int cols = m_map.GetCols();
    int rows = m_map.GetRows();
    int ts = m_map.GetTileSize();
    Color gridColor = {255, 255, 255, 30};
    for (int x = 0; x <= cols; x++)
        DrawLine(x * ts, 0, x * ts, rows * ts, gridColor);
    for (int y = 0; y <= rows; y++)
        DrawLine(0, y * ts, cols * ts, y * ts, gridColor);

    // Active-brush ghost on the hovered tile.
    if (m_map.GetGrid().InBounds(m_hoverX, m_hoverY)) {
        Vector2 wp = m_map.TileToWorld(m_hoverX, m_hoverY);
        DrawRectangle(static_cast<int>(wp.x), static_cast<int>(wp.y), ts, ts,
                      BrushTint(static_cast<int>(m_brush)));
        DrawRectangleLines(static_cast<int>(wp.x), static_cast<int>(wp.y), ts, ts, RAYWHITE);
    }

    EndMode2D();
    game.GetScreen().EndScissor();

    DrawRectangleLinesEx(m_canvasRect, 1.0f, kDefaultStyle.m_border);

    // Map name + dimensions in the canvas corner.
    Text::Draw(TextFormat("%s   %dx%d", m_meta.m_name.c_str(), m_map.GetCols(), m_map.GetRows()),
               static_cast<int>(m_canvasRect.x + 8.0f), static_cast<int>(m_canvasRect.y + 6.0f), 16, LIGHTGRAY);
}

void MapEditorState::DrawBottomBar(Game& /*game*/) {
    m_validateBtn.Draw();
    m_validateBtn.DrawLabel(18, RAYWHITE);

    bool canSave = m_lastValidateOk;
    m_saveBtn.Draw(false, canSave ? kDefaultStyle : kDisabledStyle);
    m_saveBtn.DrawLabel(18, canSave ? RAYWHITE : Color{120, 120, 120, 255});

    m_editBackBtn.Draw();
    m_editBackBtn.DrawLabel(18, RAYWHITE);
}

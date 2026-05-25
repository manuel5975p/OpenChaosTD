#include <systems/render_system.hpp>

#include <raymath.h>

void RenderSystem::DrawMap(const Map& map, Resources& assets){
    for (int y = 0; y < map.GetRows(); y++) {
        for (int x = 0; x < map.GetCols(); x++) {
            switch (map.Get(x,y).m_type) {
                case TileType::Grass:
                    DrawTexture(assets.GetTexture("tile_grass"), map.TileToWorld(x, y).x, map.TileToWorld(x, y).y, WHITE);
                    break;
                case TileType::Core:
                    DrawTexture(assets.GetTexture("tile_core"), map.TileToWorld(x, y).x, map.TileToWorld(x, y).y, WHITE);
                    break;
                case TileType::Nest:
                    DrawTexture(assets.GetTexture("tile_nest"), map.TileToWorld(x, y).x, map.TileToWorld(x, y).y, WHITE);
                    break;
                case TileType::Rock:
                    DrawTexture(assets.GetTexture("tile_rock"), map.TileToWorld(x, y).x, map.TileToWorld(x, y).y, WHITE);
                    break;
            }
        }
    }
}

void RenderSystem::DrawPaths(const Map& map){
    for (auto& path : map.GetPaths()) {
        for (size_t i=0; i < path.size(); i++) {
            if(i +1 >= path.size()) continue;
            DrawLineEx(path[i], path[i +1], 2, {200,41,55,150});
        }
    }
}

void RenderSystem::DebugDrawMap(const Map& map){
    int tileSize = map.GetTileSize();
    int halfTileSize = map.GetTileSize() /2;

    for (int y = 0; y < map.GetRows(); y++) {
        for (int x = 0; x < map.GetCols(); x++) {
            // Draw flowfield flow direction and distance
            if(map.GetPathMesh().Get(x, y).distance != std::numeric_limits<int>::max()){
                // Distance
                DrawText(TextFormat("%i", map.GetPathMesh().Get(x, y).distance), map.TileToWorld(x, y).x + 1, map.TileToWorld(x, y).y + 1, 6, BLACK);

                // Flow direction
                std::pair<int, int> end = map.GetPathMesh().Get(x, y).predecessor;;
                DrawLine(x * tileSize +halfTileSize, y * tileSize +halfTileSize, end.first * tileSize +halfTileSize, end.second * tileSize +halfTileSize, BLACK);
            }
        }
    }
}

void RenderSystem::DrawTowers(const DenseSlotMap<Tower>& towers, Resources& assets){
    for (auto& tower : towers) {
        Texture2D& texture = assets.GetTexture(tower.m_texture);
        float hw = static_cast<float>(texture.width)  / 2.0f;
        float hh = static_cast<float>(texture.height) / 2.0f;

        float flashRatio = (tower.m_stats.attackDuration > 0.0f) ? tower.m_attackFlash / tower.m_stats.attackDuration : 0.0f;
        Color tint = (flashRatio > 0.0f) ? ColorLerp(WHITE, ORANGE, flashRatio) : WHITE;
        DrawTextureV(texture, {tower.m_position.x - hw, tower.m_position.y - hh}, tint);

    }
}

void RenderSystem::DrawTowerRange(Vector2 position, float radius, Color color) {
    // Faint fill + solid outline for clear visibility
    DrawCircleV(position, radius, {color.r, color.g, color.b, 30});
    DrawCircleLinesV(position, radius, color);
}

void RenderSystem::DrawRangeIndicator(DenseSlotMap<Tower>::Key selectedKey, const Map& map, const DenseSlotMap<Tower>& towers, Vector2 mouseWorld) {
    if (selectedKey != DenseSlotMap<Tower>::INVALID_KEY) {
        if (const Tower* t = towers.Get(selectedKey))
            DrawTowerRange(t->m_position, t->m_stats.radius, {255, 200, 50, 220});
        return;
    }

    // Show range of whichever placed tower the mouse is hovering
    int hx, hy;
    if (map.WorldToTile(mouseWorld, hx, hy)) {
        const Tile& tile = map.Get(hx, hy);
        if (tile.m_towerKey != DenseSlotMap<Tower>::INVALID_KEY) {
            if (const Tower* t = towers.Get(tile.m_towerKey))
                DrawTowerRange(t->m_position, t->m_stats.radius, {255, 255, 255, 80});
        }
    }
}

void RenderSystem::DrawGhostTower(Vector2 position, float radius, Texture2D& texture) {
    float hw = static_cast<float>(texture.width)  / 2.0f;
    float hh = static_cast<float>(texture.height) / 2.0f;
    DrawTextureV(texture, {position.x - hw, position.y - hh}, {255, 255, 255, 140});
    // Faint fill + outline to match the selected range style
    DrawCircleV(position, radius, {255, 255, 255, 15});
    DrawCircleLinesV(position, radius, {255, 255, 255, 140});
}

void RenderSystem::DrawEnemies(const DenseSlotMap<Enemy>& enemies, Resources& assets) {
    for (auto& enemy : enemies) {
        Texture2D& texture = assets.GetTexture(enemy.m_texture);
        float hw = static_cast<float>(texture.width)  / 2.0f;
        float hh = static_cast<float>(texture.height) / 2.0f;

        DrawTextureV(texture, {enemy.m_position.x - hw, enemy.m_position.y - hh}, WHITE);

        // Health bar: 24px wide, 4px tall, floats above the sprite
        DrawHealthBar({enemy.m_position.x, enemy.m_position.y + hh + 2.0f}, enemy.m_currentHealth, enemy.m_maxHealth, 20.0f, 4.0f );
    }
}

void RenderSystem::DrawVfx(const std::vector<VfxEffect>& vfx) {
    for (const auto& v : vfx) {
        float t = v.Progress();

        switch (v.m_style) {
            case VfxStyle::Beam:
                // Double layer: wide soft outer + thin bright inner gives perceived glow
                for (const auto& target : v.m_targetPositions) {
                    DrawLineEx(v.m_origin, target, 4.0f, ColorAlpha(v.m_color, t * 0.4f));
                    DrawLineEx(v.m_origin, target, 2.0f, ColorAlpha(v.m_color, t * 0.9f));
                }
                break;
            case VfxStyle::Zap:
                for (const auto& target : v.m_targetPositions)
                    DrawLineEx(v.m_origin, target, 1.5f, ColorAlpha(v.m_color, t));
                break;
            case VfxStyle::Burst:
                // Circle expands outward as it fades — no line drawn
                for (const auto& target : v.m_targetPositions) {
                    float r = (1.0f - t) * 18.0f;
                    DrawCircleV(target, r, ColorAlpha(v.m_color, t * 0.8f));
                    DrawCircleLinesV(target, r, ColorAlpha(v.m_color, t));
                }
                break;
            case VfxStyle::Ring:
                // Ring expands from tower outward to full radius
                float r = (1.0f - t) * v.m_radius;
                DrawCircleV(v.m_origin, r, ColorAlpha(v.m_color, t * 0.12f));
                DrawCircleLinesV(v.m_origin, r, ColorAlpha(v.m_color, t * 0.9f));
                break;
        }
    }
}

void RenderSystem::DebugDrawEnemies(const DenseSlotMap<Enemy>& enemies) {
    for (auto& enemy : enemies) {
        DrawText(
            TextFormat("%.2f", enemy.m_progress),
            static_cast<int>(enemy.m_position.x) + 6,
            static_cast<int>(enemy.m_position.y) - 18,
            8, LIME
        );
    }
}

void RenderSystem::CenterCamera(Map& map, Screen& renderer){
    camera.target = {-static_cast<float>(renderer.GetGameWidth()) / 2.f, -static_cast<float>(renderer.GetGameHeight()) / 2.f};
    camera.offset = {-(map.GetCols() * map.GetTileSize()) / 2.f, -(map.GetRows() * map.GetTileSize()) / 2.f};
    camera.zoom = 1.0f;
}

void RenderSystem::ControlCamera(float& dt, Input& input){
    // ------------------------------
    // Moving Camera
    // ------------------------------
    Vector2 direction{0,0};

    // Move camera with keyboard
    if(input.IsDown("Up")) direction.y ++;
    if(input.IsDown("Down")) direction.y --;
    if(input.IsDown("Right")) direction.x --;
    if(input.IsDown("Left")) direction.x ++;
    direction *= 300 * dt;

    // Move camera by draging
    if(input.IsDown("DragCamera")){
        direction = (input.GetMousePosition() -mousePositionLast) / camera.zoom;
    }

    camera.target -= direction;

    // Update mousePositionLast
    mousePositionLast = input.GetMousePosition();

    // ------------------------------
    // Zooming Camera
    // ------------------------------
    float wheel = input.GetMouseWheelDelta();
    if(wheel != 0){
        Vector2 mouseScreen = input.GetMousePosition();

        // 1. Where in the world is the mouse RIGHT NOW?
        Vector2 mouseWorld = input.GetWorldMousePosition(camera);

        // 2. Shift offset to mouse (makes mouse the anchor)
        camera.offset = mouseScreen;
        camera.target = mouseWorld;

        // 3. Apply zoom
        zoomIndex += wheel;
        zoomIndex = Clamp(zoomIndex, 0, static_cast<int>(sizeof(zoomLevel) / sizeof(zoomLevel[0])) -1);

        camera.zoom = zoomLevel[zoomIndex];
    }
}

// Helper: interpolate color green -> yellow -> red based on health ratio
static Color HealthBarColor(float ratio) {
    if (ratio > 0.5f) {
        // green to yellow
        float t = (ratio - 0.5f) * 2.0f;
        return ColorLerp(YELLOW, GREEN, t);
    } else {
        // yellow to red
        float t = ratio * 2.0f;
        return ColorLerp(RED, YELLOW, t);
    }
}

void RenderSystem::DrawHealthBar(Vector2 worldPos, float current, float max, float width, float height) {
    float ratio = Clamp(current / max, 0.0f, 1.0f);
    float x = worldPos.x - width / 2.0f;
    float y = worldPos.y - height / 2.0f;
    float roundness = 0.2f;
    int segments = 8;

    // BackgroundBar
    DrawRectangleRounded( {x, y, width, height}, roundness, segments, {40, 40, 40, 255});
    
    // HealthBar
    float pad = 1.0f;
    float fillWidth = (width - pad * 2) * ratio;
    if (fillWidth > 0.0f) {
        DrawRectangleRec( {x + pad, y + pad, fillWidth, height - pad * 2}, HealthBarColor(ratio));
    }
}
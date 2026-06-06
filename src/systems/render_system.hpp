#pragma once

#include <world/map.hpp>
#include <engine/core/resources.hpp>
#include <engine/core/input.hpp>
#include <engine/core/screen.hpp>
#include <world/tower.hpp>
#include <world/enemy.hpp>
#include <world/attack.hpp>
#include <engine/lib/dense_slotmap.hpp>

class RenderSystem{
public:
    // Draw calls
    void DrawMap(const Map& map, Resources& assets);
    void DrawPaths(const Map& map);
    void DebugDrawMap(const Map& Map);
    void DebugDrawEnemies(const DenseSlotMap<Enemy>& enemies);
    void DrawTowers(const DenseSlotMap<Tower>& towers, Resources& assets);
    void DrawTowerRange(Vector2 position, float radius, Color color);
    void DrawRangeIndicator(DenseSlotMap<Tower>::Key selectedKey, const Map& map, const DenseSlotMap<Tower>& towers, Vector2 mouseWorld);
    void DrawGhostTower(Vector2 position, float radius, Texture2D& texture);
    void DrawEnemies(const DenseSlotMap<Enemy>& enemies, Resources& assets);
    // Draw an enemy sprite scaled (aspect-preserving) to fit `dest`; reads the texture from Resources.
    // Used by the WaveHUD enemy cards. No-op if the key is missing.
    void DrawEnemyIcon(const std::string& textureKey, Resources& assets, Rectangle dest) const;
    void DrawAttacks(const std::vector<Attack>& attacks);

    void CenterCamera(Map& map, Screen& renderer);
    void ControlCamera(float& dt, Input& input);

    // Access
    const Camera2D& GetCamera(){return camera;}

private:
    Vector2 mousePositionLast;

    Camera2D camera;
    int zoomIndex = 1;
    float zoomLevel[4] = {0.5, 1, 2, 4};

    void DrawHealthBar(Vector2 worldPos, float current, float max, float width, float height);
};
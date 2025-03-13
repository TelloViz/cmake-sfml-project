#pragma once
#define _USE_MATH_DEFINES

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include <cmath>
#include <functional>

class TerrainGenerator {
public:
    // Callback type for terrain updates
    using UpdateCallback = std::function<void()>;

    struct Cave {
        sf::Vector2f position;
        float rotation;      // Base rotation of the cave
        float scaleVariant;  // Individual scale modifier
        float noiseOffset;   // Offset for noise sampling
    };

    explicit TerrainGenerator(unsigned int width, unsigned int height);

    // Main generation method
    sf::RenderTexture& generateTerrain();

    // Register callback for terrain updates
    void onTerrainUpdated(UpdateCallback callback) { m_updateCallback = callback; }

    // Setters
    void setPointCount(int count);
    void setBaseRadius(int radius);
    void setHorizontalStretch(float stretch);
    void setNoiseFrequency(float freq);
    void setNoiseAmplitude(float amp);
    void setBlobCount(int count);
    void setBlobSpacing(float spacing);
    void setCavesEnabled(bool enabled);
    void setCaveScale(float scale);
    void setCaveNoiseFrequency(float freq);
    void setCaveNoiseAmplitude(float amp);
    void setCaveCount(int count);
    void setCavePointCount(int count);
    void setSelectedCaveIndex(int index);

    // Getters
    int getPointCount() const { return m_pointCount; }
    int getBaseRadius() const { return m_baseRadius; }
    float getHorizontalStretch() const { return m_horizontalStretch; }
    float getNoiseFrequency() const { return m_noiseFrequency; }
    float getNoiseAmplitude() const { return m_noiseAmplitude; }
    int getBlobCount() const { return m_blobCount; }
    float getBlobSpacing() const { return m_blobSpacing; }
    bool getCavesEnabled() const { return m_cavesEnabled; }
    float getCaveScale() const { return m_caveScale; }
    float getCaveNoiseFrequency() const { return m_caveNoiseFrequency; }
    float getCaveNoiseAmplitude() const { return m_caveNoiseAmplitude; }
    int getCaveCount() const { return m_caveCount; }
    int getCavePointCount() const { return m_cavePointCount; }
    int getSelectedCaveIndex() const { return m_selectedCaveIndex; }

    // Cave manipulation
    void updateSelectedCave(float scale, float rotation, float noiseOffset);
    Cave getSelectedCaveProperties() const;
    void regenerateCavePositions();
    void regenerateSelectedCavePosition();

private:
    void drawBlob(sf::RenderTexture& target);
    void drawMultiBlob(sf::RenderTexture& target);
    void subtractBlob(sf::RenderTexture& target, const sf::Vector2f& center);
    float noise2D(float x, float y);
    float fade(float t);
    float lerp(float t, float a, float b);
    float grad(int hash, float x, float y);
    void notifyUpdate() { if (m_updateCallback) m_updateCallback(); }

    unsigned int m_width;
    unsigned int m_height;
    int m_pointCount{20};
    int m_baseRadius;
    float m_horizontalStretch{1.0f};
    float m_noiseFrequency{1.0f};
    float m_noiseAmplitude{1.0f};
    int m_blobCount{1};
    float m_blobSpacing{1.5f};
    bool m_cavesEnabled{true};
    float m_caveScale{0.3f};
    float m_caveNoiseFrequency{2.0f};
    float m_caveNoiseAmplitude{1.0f};
    int m_caveCount{0};
    int m_cavePointCount{20};
    std::vector<Cave> m_caves;
    int m_selectedCaveIndex{-1};
    //std::optional<sf::RenderTexture> m_terrainTexture;
    sf::RenderTexture m_terrainTexture;
    std::mt19937 m_rng{std::random_device{}()};
    UpdateCallback m_updateCallback;
};

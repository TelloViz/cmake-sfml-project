#include "../include/TerrainGenerator.hpp"
#include <cmath>
#include <algorithm>
#include <random>

TerrainGenerator::TerrainGenerator(unsigned int w, unsigned int h) 
    : m_width(w)
    , m_height(h)
    , m_terrainTexture(sf::Vector2u{w, h})  // Direct construction
{
    m_baseRadius = std::min(m_width, m_height) / 3;
    m_terrainTexture.clear(sf::Color::Transparent);  // Use . instead of ->
    m_blobCount = 1;  // Initialize blob count
    m_caveCount = 0;  // Initialize cave count
    regenerateCavePositions();
}

void TerrainGenerator::setPointCount(int count) {
    if (m_pointCount != count) {
        m_pointCount = count;
        notifyUpdate();
    }
}

void TerrainGenerator::setBaseRadius(int radius) {
    if (m_baseRadius != radius) {
        m_baseRadius = radius;
        notifyUpdate();
    }
}

void TerrainGenerator::setHorizontalStretch(float stretch) {
    if (m_horizontalStretch != stretch) {
        m_horizontalStretch = stretch;
        notifyUpdate();
    }
}

void TerrainGenerator::setNoiseFrequency(float freq) {
    if (m_noiseFrequency != freq) {
        m_noiseFrequency = freq;
        notifyUpdate();
    }
}

void TerrainGenerator::setNoiseAmplitude(float amp) {
    if (m_noiseAmplitude != amp) {
        m_noiseAmplitude = amp;
        notifyUpdate();
    }
}

void TerrainGenerator::setBlobCount(int count) {
    if (m_blobCount != count) {
        m_blobCount = count;
        regenerateCavePositions();
        notifyUpdate();
    }
}

void TerrainGenerator::setBlobSpacing(float spacing) {
    if (m_blobSpacing != spacing) {
        m_blobSpacing = spacing;
        notifyUpdate();
    }
}

void TerrainGenerator::setCavesEnabled(bool enabled) {
    if (m_cavesEnabled != enabled) {
        m_cavesEnabled = enabled;
        if (enabled) {
            regenerateCavePositions();
        } else {
            m_caves.clear();
        }
        notifyUpdate();
    }
}

void TerrainGenerator::setCaveScale(float scale) {
    if (m_caveScale != scale) {
        m_caveScale = scale;
        notifyUpdate();
    }
}

void TerrainGenerator::setCaveNoiseFrequency(float freq) {
    if (m_caveNoiseFrequency != freq) {
        m_caveNoiseFrequency = freq;
        notifyUpdate();
    }
}

void TerrainGenerator::setCaveNoiseAmplitude(float amp) {
    if (m_caveNoiseAmplitude != amp) {
        m_caveNoiseAmplitude = amp;
        notifyUpdate();
    }
}

void TerrainGenerator::setCaveCount(int count) {
    if (m_caveCount != count) {
        int oldCount = m_caveCount;
        m_caveCount = count;

        // Always regenerate caves when count changes
        m_caves.clear();  // Clear existing caves
        
        for (int i = 0; i < count; i++) {
            Cave cave;
            int minX = static_cast<int>(m_width * 0.2f);
            int maxX = static_cast<int>(m_width * 0.8f);
            int minY = static_cast<int>(m_height * 0.3f);
            int maxY = static_cast<int>(m_height * 0.7f);
            
            std::uniform_int_distribution<int> xDist(minX, maxX);
            std::uniform_int_distribution<int> yDist(minY, maxY);
            std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * static_cast<float>(M_PI));
            std::uniform_real_distribution<float> scaleDist(0.8f, 1.2f);
            std::uniform_real_distribution<float> noiseDist(0.0f, 10.0f);
            
            cave.position = sf::Vector2f(xDist(m_rng), yDist(m_rng));
            cave.rotation = angleDist(m_rng);
            cave.scaleVariant = scaleDist(m_rng);
            cave.noiseOffset = noiseDist(m_rng);
            
            m_caves.push_back(cave);
        }

        if (m_selectedCaveIndex >= m_caves.size()) {
            m_selectedCaveIndex = m_caves.empty() ? -1 : m_caves.size() - 1;
        }
        notifyUpdate();
    }
}

void TerrainGenerator::setCavePointCount(int count) {
    if (m_cavePointCount != count) {
        m_cavePointCount = count;
        notifyUpdate();
    }
}

void TerrainGenerator::setSelectedCaveIndex(int index) {
    if (m_selectedCaveIndex != index && index >= -1 && index < m_caves.size()) {
        m_selectedCaveIndex = index;
        notifyUpdate();
    }
}

TerrainGenerator::Cave TerrainGenerator::getSelectedCaveProperties() const {
    if (m_selectedCaveIndex >= 0 && m_selectedCaveIndex < m_caves.size()) {
        return m_caves[m_selectedCaveIndex];
    }
    return Cave(); // Return default cave if none selected
}

void TerrainGenerator::updateSelectedCave(float scale, float rotation, float noiseOffset) {
    if (m_selectedCaveIndex >= 0 && m_selectedCaveIndex < m_caves.size()) {
        Cave& cave = m_caves[m_selectedCaveIndex];
        cave.scaleVariant = scale;
        cave.rotation = rotation;
        cave.noiseOffset = noiseOffset;
        notifyUpdate();
    }
}

sf::RenderTexture& TerrainGenerator::generateTerrain() {
    m_terrainTexture.clear(sf::Color::Transparent);  // Use . instead of ->
    drawMultiBlob(m_terrainTexture);  // Pass direct reference
    m_terrainTexture.display();
    return m_terrainTexture;
}

void TerrainGenerator::drawBlob(sf::RenderTexture& target) {
    sf::ConvexShape blob;
    blob.setPointCount(m_pointCount);
    blob.setFillColor(sf::Color::Black);
    blob.setOutlineThickness(0);

    float centerX = m_width / 2.0f;
    float centerY = m_height / 2.0f;

    for (int i = 0; i < m_pointCount; i++) {
        float angle = (2 * M_PI * i) / m_pointCount;
        float noiseX = std::cos(angle) * m_noiseFrequency;
        float noiseY = std::sin(angle) * m_noiseFrequency;
        float variation = noise2D(noiseX, noiseY) * m_noiseAmplitude * m_baseRadius * 0.5f;
        
        float radius = m_baseRadius + variation;
        float x = centerX + (radius * std::cos(angle) * m_horizontalStretch);
        float y = centerY + radius * std::sin(angle);
        blob.setPoint(i, sf::Vector2f(x, y));
    }

    target.draw(blob);
}

void TerrainGenerator::drawMultiBlob(sf::RenderTexture& target) {
    // Calculate total width needed for all blobs
    float totalWidth = (m_blobCount - 1) * m_baseRadius * m_blobSpacing;
    float startX = (m_width - totalWidth) / 2.0f;
    
    // Draw main surface blobs - always draw at least one blob
    for (int i = 0; i < std::max(1, m_blobCount); i++) {
        float xPos = startX + (i * m_baseRadius * m_blobSpacing);
        sf::Vector2f center(xPos, m_height/2.0f);
        
        sf::ConvexShape blob;
        blob.setPointCount(m_pointCount);
        blob.setFillColor(sf::Color::Black);
        blob.setOutlineThickness(0);
        
        for (int j = 0; j < m_pointCount; j++) {
            float angle = (2 * M_PI * j) / m_pointCount;
            float noiseX = (std::cos(angle) + i) * m_noiseFrequency;
            float noiseY = std::sin(angle) * m_noiseFrequency;
            float variation = noise2D(noiseX, noiseY) * m_noiseAmplitude * m_baseRadius * 0.5f;
            
            float radius = m_baseRadius + variation;
            float x = center.x + (radius * std::cos(angle) * m_horizontalStretch);
            float y = center.y + radius * std::sin(angle);
            blob.setPoint(j, sf::Vector2f(x, y));
        }
        
        target.draw(blob);
    }
    
    if (m_cavesEnabled && !m_caves.empty()) {
        // Create stencil for cave cutouts with correct blend mode
        sf::RenderStates cutoutState;
        cutoutState.blendMode = sf::BlendMode(
            sf::BlendMode::Factor::DstColor,
            sf::BlendMode::Factor::OneMinusSrcColor
        );
        
        for (const Cave& cave : m_caves) {
            sf::ConvexShape caveBlob;
            caveBlob.setPointCount(m_cavePointCount);
            caveBlob.setFillColor(sf::Color::White);
            caveBlob.setOutlineThickness(0);
            
            float caveRadius = m_baseRadius * m_caveScale * cave.scaleVariant;
            
            for (int j = 0; j < m_cavePointCount; j++) {
                float angle = (2 * M_PI * j) / m_cavePointCount + cave.rotation;
                float noiseX = std::cos(angle) * m_caveNoiseFrequency + cave.noiseOffset;
                float noiseY = std::sin(angle) * m_caveNoiseFrequency + cave.noiseOffset;
                float variation = noise2D(noiseX, noiseY) * m_caveNoiseAmplitude * caveRadius * 0.5f;
                
                float radius = caveRadius + variation;
                float x = cave.position.x + radius * std::cos(angle);
                float y = cave.position.y + radius * std::sin(angle);
                caveBlob.setPoint(j, sf::Vector2f(x, y));
            }
            
            target.draw(caveBlob, cutoutState);
        }
    }
}

void TerrainGenerator::subtractBlob(sf::RenderTexture& target, const sf::Vector2f& center) {
    sf::ConvexShape blob;
    blob.setPointCount(m_pointCount);
    blob.setFillColor(sf::Color::White);
    blob.setOutlineThickness(0);

    float caveRadius = m_baseRadius * 0.3f;
    
    for (int i = 0; i < m_pointCount; i++) {
        float angle = (2 * M_PI * i) / m_pointCount;
        
        float noiseX = std::cos(angle) * m_noiseFrequency * 2;
        float noiseY = std::sin(angle) * m_noiseFrequency * 2;
        float variation = noise2D(noiseX, noiseY) * m_noiseAmplitude * caveRadius * 0.5f;
        
        float radius = caveRadius + variation;
        float x = center.x + radius * std::cos(angle);
        float y = center.y + radius * std::sin(angle);
        blob.setPoint(i, sf::Vector2f(x, y));
    }
    
    sf::RenderStates cutoutState;
    cutoutState.blendMode = sf::BlendMode(
        sf::BlendMode::Factor::DstColor,  // Replace Zero
        sf::BlendMode::Factor::OneMinusSrcColor  // Replace One
    );
    target.draw(blob, cutoutState);
}

// Modify regenerateCavePositions to only regenerate if caves vector is empty
void TerrainGenerator::regenerateCavePositions() {
    if (!m_cavesEnabled || !m_caves.empty()) {
        return;
    }

    std::uniform_real_distribution<float> rotDist(0.0f, 2.0f * static_cast<float>(M_PI));
    std::uniform_real_distribution<float> scaleDist(0.8f, 1.2f);
    std::uniform_real_distribution<float> noiseDist(0.0f, 10.0f);

    for (int i = 0; i < m_caveCount; i++) {
        Cave cave;
        int minX = static_cast<int>(m_width * 0.2f);
        int maxX = static_cast<int>(m_width * 0.8f);
        int minY = static_cast<int>(m_height * 0.3f);
        int maxY = static_cast<int>(m_height * 0.7f);
        
        std::uniform_int_distribution<int> xDist(minX, maxX);
        std::uniform_int_distribution<int> yDist(minY, maxY);
        
        cave.position = sf::Vector2f(xDist(m_rng), yDist(m_rng));
        cave.rotation = rotDist(m_rng);
        cave.scaleVariant = scaleDist(m_rng);
        cave.noiseOffset = noiseDist(m_rng);
        
        m_caves.push_back(cave);
    }
    notifyUpdate();
}

void TerrainGenerator::regenerateSelectedCavePosition() {
    if (m_selectedCaveIndex >= 0 && m_selectedCaveIndex < m_caves.size()) {
        Cave& cave = m_caves[m_selectedCaveIndex];
        
        int minX = static_cast<int>(m_width * 0.2f);
        int maxX = static_cast<int>(m_width * 0.8f);
        int minY = static_cast<int>(m_height * 0.3f);
        int maxY = static_cast<int>(m_height * 0.7f);
        
        std::uniform_int_distribution<int> xDist(minX, maxX);
        std::uniform_int_distribution<int> yDist(minY, maxY);
        
        cave.position = sf::Vector2f(
            xDist(m_rng),
            yDist(m_rng)
        );

        notifyUpdate();
    }
}

float TerrainGenerator::noise2D(float x, float y) {
    // Implementation of 2D Perlin noise
    static const unsigned char permutation[256] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
    };

    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;
    x -= floor(x);
    y -= floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = permutation[X] + Y;
    int B = permutation[X + 1] + Y;

    return lerp(v, 
        lerp(u, grad(permutation[A], x, y), 
                grad(permutation[B], x - 1, y)),
        lerp(u, grad(permutation[A + 1], x, y - 1),
                grad(permutation[B + 1], x - 1, y - 1)));
}

float TerrainGenerator::fade(float t) {
    // Fade function as defined by Ken Perlin
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float TerrainGenerator::lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float TerrainGenerator::grad(int hash, float x, float y) {
    // Convert low 4 bits of hash code into 12 gradient directions
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : 0;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

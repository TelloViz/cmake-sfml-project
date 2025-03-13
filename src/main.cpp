#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <iostream>
#include "TerrainGenerator.hpp"

int main() {
    std::cout << "Starting application...\n";
    
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "Cave Generation Demo");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        std::cout << "Failed to initialize ImGui-SFML\n";
        return -1;
    }

    std::cout << "Window created and ImGui initialized\n";

    // Create terrain generator with window size
    TerrainGenerator terrainGen(window.getSize().x, window.getSize().y);
    
    sf::Clock deltaClock;
    while (window.isOpen()) {
        // Handle all events in queue
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        // ImGui controls window
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_FirstUseEver);
        ImGui::Begin("Cave Generator Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        // Basic shape controls
        if (ImGui::CollapsingHeader("Basic Shape")) {
            int pointCount = terrainGen.getPointCount();
            if (ImGui::SliderInt("Point Count", &pointCount, 3, 100)) {
                terrainGen.setPointCount(pointCount);
            }

            int baseRadius = terrainGen.getBaseRadius();
            if (ImGui::SliderInt("Base Radius", &baseRadius, 10, 300)) {
                terrainGen.setBaseRadius(baseRadius);
            }

            float stretch = terrainGen.getHorizontalStretch();
            if (ImGui::SliderFloat("Horizontal Stretch", &stretch, 0.1f, 3.0f)) {
                terrainGen.setHorizontalStretch(stretch);
            }
        }

        // Noise controls
        if (ImGui::CollapsingHeader("Noise Parameters")) {
            float freq = terrainGen.getNoiseFrequency();
            if (ImGui::SliderFloat("Noise Frequency", &freq, 0.1f, 5.0f)) {
                terrainGen.setNoiseFrequency(freq);
            }

            float amp = terrainGen.getNoiseAmplitude();
            if (ImGui::SliderFloat("Noise Amplitude", &amp, 0.0f, 2.0f)) {
                terrainGen.setNoiseAmplitude(amp);
            }
        }

        // Blob controls
        if (ImGui::CollapsingHeader("Blob Controls")) {
            int blobCount = terrainGen.getBlobCount();
            if (ImGui::SliderInt("Blob Count", &blobCount, 1, 10)) {
                terrainGen.setBlobCount(blobCount);
            }

            float blobSpacing = terrainGen.getBlobSpacing();
            if (ImGui::SliderFloat("Blob Spacing", &blobSpacing, 0.5f, 3.0f)) {
                terrainGen.setBlobSpacing(blobSpacing);
            }
        }

        // Cave controls
        if (ImGui::CollapsingHeader("Cave System")) {
            bool cavesEnabled = terrainGen.getCavesEnabled();
            if (ImGui::Checkbox("Enable Caves", &cavesEnabled)) {
                terrainGen.setCavesEnabled(cavesEnabled);
            }

            if (cavesEnabled) {
                // Global cave parameters
                int caveCount = terrainGen.getCaveCount();
                if (ImGui::SliderInt("Cave Count", &caveCount, 0, 10)) {
                    terrainGen.setCaveCount(caveCount);
                }

                int cavePointCount = terrainGen.getCavePointCount();
                if (ImGui::SliderInt("Cave Point Count", &cavePointCount, 3, 100)) {
                    terrainGen.setCavePointCount(cavePointCount);
                }

                float caveScale = terrainGen.getCaveScale();
                if (ImGui::SliderFloat("Global Cave Scale", &caveScale, 0.1f, 1.0f)) {
                    terrainGen.setCaveScale(caveScale);
                }

                float caveNoiseFreq = terrainGen.getCaveNoiseFrequency();
                if (ImGui::SliderFloat("Cave Noise Frequency", &caveNoiseFreq, 0.1f, 5.0f)) {
                    terrainGen.setCaveNoiseFrequency(caveNoiseFreq);
                }

                float caveNoiseAmp = terrainGen.getCaveNoiseAmplitude();
                if (ImGui::SliderFloat("Cave Noise Amplitude", &caveNoiseAmp, 0.0f, 2.0f)) {
                    terrainGen.setCaveNoiseAmplitude(caveNoiseAmp);
                }

                // Individual cave editing
                if (ImGui::TreeNode("Edit Individual Cave")) {
                    int selectedCave = terrainGen.getSelectedCaveIndex();
                    if (ImGui::SliderInt("Selected Cave", &selectedCave, -1, caveCount - 1)) {
                        terrainGen.setSelectedCaveIndex(selectedCave);
                    }

                    if (selectedCave >= 0) {
                        auto cave = terrainGen.getSelectedCaveProperties();
                        float scale = cave.scaleVariant;
                        float rotation = cave.rotation;
                        float noiseOffset = cave.noiseOffset;

                        bool modified = false;
                        modified |= ImGui::SliderFloat("Individual Scale", &scale, 0.5f, 2.0f);
                        modified |= ImGui::SliderAngle("Rotation", &rotation);
                        modified |= ImGui::SliderFloat("Noise Offset", &noiseOffset, 0.0f, 10.0f);

                        if (modified) {
                            terrainGen.updateSelectedCave(scale, rotation, noiseOffset);
                        }

                        if (ImGui::Button("Regenerate Position")) {
                            terrainGen.regenerateSelectedCavePosition();
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }

        if (ImGui::CollapsingHeader("Terrain Statistics")) {
            auto stats = terrainGen.calculateStats();
            ImGui::Text("Visible Terrain Pixels: %u", stats.visibleTerrainPixels);
            ImGui::Text("Terrain Coverage: %.1f%%", stats.terrainCoverage);
            ImGui::ProgressBar(stats.terrainCoverage / 100.0f);
        }

        // Export controls
        if (ImGui::CollapsingHeader("Export")) {
            static char filename[128] = "terrain.png";
            static TerrainGenerator::ExportSettings exportSettings;
            
            ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
            
            // Background transparency option
            ImGui::Checkbox("Transparent Background", &exportSettings.transparentBackground);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("If checked, areas outside the terrain will be transparent");
                ImGui::EndTooltip();
            }
            
            // Cave transparency option
            ImGui::Checkbox("Transparent Caves", &exportSettings.transparentCaves);
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("If checked, caves will be holes in the terrain\n"
                           "If unchecked, caves will be white");
                ImGui::EndTooltip();
            }
            
            if (ImGui::Button("Save Terrain")) {
                if (terrainGen.saveToFile(filename, exportSettings)) {
                    ImGui::OpenPopup("Save Success");
                } else {
                    ImGui::OpenPopup("Save Failed");
                }
            }
            
            // Popup modals for feedback
            if (ImGui::BeginPopupModal("Save Success", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Terrain saved successfully!");
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginPopupModal("Save Failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Failed to save terrain!");
                if (ImGui::Button("OK")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::End();

        // Render
        window.clear(sf::Color::White);
        
        // Draw terrain - center it in window
        sf::RenderTexture& terrain = terrainGen.generateTerrain();
        sf::Sprite terrainSprite(terrain.getTexture());
        // Center the sprite using Vector2f
        terrainSprite.setPosition(sf::Vector2f(
            (window.getSize().x - terrain.getSize().x) / 2.0f,
            (window.getSize().y - terrain.getSize().y) / 2.0f
        ));
        window.draw(terrainSprite);
        
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    std::cout << "Application shutting down\n";
    return 0;
}
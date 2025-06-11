/**
 * @author neucrack@sipeed
 * @copyright Sipeed Ltd 2023-
 * @license Apache 2.0
 * @update 2025.6.9: Add cmap def.
 */

 #pragma once

 #include <vector>
 #include <string>
 #include <map>
 #include <stdint.h>
 #include <stdexcept>
 #include "maix_image_color.hpp"

 namespace maix::image
 {

    /**
     * Image formats
     * @attention for MaixPy firmware developers, update this enum will also need to update the fmt_size and fmt_names too !!!
     * @maixpy maix.image.CMap
     */
    enum class CMap
    {
        // Sequential (Perceptually uniform linear gradients)
        TURBO = 0,           // Google-designed smooth, bright colormap; good Jet replacement for heatmaps
        VIRIDIS,             // Default perceptually uniform colormap; colorblind-friendly and widely recommended
        INFERNO,             // High-contrast, perceptually uniform; ideal for dark backgrounds and scientific data
        PLASMA,              // Bright and visually appealing; perceptually uniform and colorblind-friendly
        CIVIDIS,             // Grayscale-compatible and colorblind-safe; designed for visual accessibility
        CUBEHELIX,           // Perceptually uniform and printable in grayscale; spirals through color space
        MAGMA,               // Dark background-optimized; smooth gradient with good visibility in low-light visuals
        TWILIGHT,            // Cyclic gradient; useful for circular data such as angles or phase data
        TWILIGHT_SHIFTED,    // Phase-shifted version of Twilight; same use case but shifted zero point
        GREYS,               // Monotonic black-to-white gradient; useful for basic grayscale visualization

        // Traditional (Legacy and visually distinct maps)
        JET,                 // Classic but non-uniform RGB colormap; visually vibrant but not perceptually linear

        // Diverging (Center-out maps for relative differences)
        COOLWARM,            // Blue-white-red diverging map; ideal for visualizing deviation from a neutral center
        RDYBU,               // Red-white-blue diverging map; suitable for highlighting opposing directions/values

        // Qualitative (For categorical or class label data)
        SET1,                // High-contrast colors; up to 9 distinct categories, ideal for class segmentation
        TAB10,               // Matplotlib default for 10-category labeling; visually distinct and colorblind-friendly
        TAB20,               // Matplotlib default for 20-category labeling; visually distinct and colorblind-friendly

        // thermal
        THERMAL_WHITE_HOT,   // Hotter whiter; used thermal imaging mode due to its natural contrast.
        THERMAL_BLACK_HOT,   // Hotter darker; Often used in security and tactical applications for better object detection.
        THERMAL_RED_HOT,     // Hotter red，cooler areas dark； Emphasizes hotspots, useful for quick temperature anomaly detection.
        THERMAL_WHITE_HOT_SD, /* White Hot SD (Smooth Detail): Enhanced White Hot with improved local contrast and smoother tone transitions.
                                 Helps distinguish fine temperature differences in subtle thermal scenes.
                              */
        THERMAL_BLACK_HOT_SD, /* Black Hot SD (Smooth Detail): Enhanced Black Hot with detail-preserving smoothing and finer thermal gradients.
                                 Improves readability of dark-hot scenes with less visual noise.
                              */
        THERMAL_RED_HOT_SD,   /* Red Hot SD (Smooth Detail): Enhanced Red Hot with more gradual color changes and smoother heat spot highlighting.
                                 Offers better heat trace visualization with reduced abrupt transitions.
                              */
        THERMAL_IRONBOW,      /* Ironbow: A classic thermal colormap that uses a smooth gradient from dark to bright (black → red → yellow → white).
                                 Commonly used in industrial and medical thermography for better visualization.
                              */
        THERMAL_NIGHT,      /* Night: A low-brightness thermal mode optimized for night operations or low-light display environments.
                                 Typically enhances visibility in high-contrast night-time scenarios.
                              */

        // Special / Custom
        GITHUB_GREEN,        // GitHub-themed green-based palette; custom usage such as contribution graphs

        MAX                 // Total count of valid colormaps (not a colormap itself)
    };

    /**
    * Get the string representation of a colormap enum value.
    * @param cmap Colormap enum value, @see image::CMap
    * @return Corresponding colormap name as a string.
    * @maixpy maix.image.cmap_to_str
    */
    std::string cmap_to_str(image::CMap cmap);

    /**
     * Get the colormap enum value from a string name.
     * @param name Name of the colormap (case-insensitive).
     * @return Corresponding colormap enum value, or throws if name is invalid.
     * @maixpy maix.image.cmap_from_str
     */
    image::CMap cmap_from_str(const std::string& name);

    /**
     * Get a list of all available colormap name strings.
     * @param claasify cmaps for classify, wihch cmap size not equal to 256.
     * @return Vector of all supported colormap name strings.
     * @maixpy maix.image.cmap_strs
     */
     std::vector<std::string> cmap_strs(bool classify = false);

    /**
     * Get the mapped color of a grayscale value under a specific colormap.
     * @param gray Grayscale value in [0, 255].
     * @param cmap Colormap enum value to map the grayscale to color.
     * @return The mapped image::Color.
     * @maixpy maix.image.cmap_color
     */
    image::Color cmap_color(uint8_t gray, image::CMap cmap);

    /**
     * Get all 256 mapped colors of a colormap.
     * @param cmap Colormap enum value.
     * @return Vector of 256 image::Color values corresponding to grayscale values [0, 255].
     *         Return value will alloc data, you need to delete it after use in C++.
     * @maixpy maix.image.cmap_colors
     */
     std::vector<image::Color> cmap_colors(image::CMap cmap);

    /**
     * Get all 256 mapped RGB colors of a colormap.
     * @param cmap Colormap enum value.
     * @return Vector of 256 RGB arrays (each array has 3 uint8_t: R, G, B).
     *         Return value is a internal table refrence.
     * @maixpy maix.image.cmap_colors_rgb
     */
    const std::vector<std::array<uint8_t, 3>> &cmap_colors_rgb(image::CMap cmap);



}; // namespace maix::image



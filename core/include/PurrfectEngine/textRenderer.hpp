#include <iostream>
#include <ft2build.h>
#include <string>
#include <fontManager.hpp>
#include "renderer.hpp"
#include FT_FREETYPE_H

class TextRenderer {
public:
    TextRenderer(vkRenderer* renderer, FontManager* fontManager) : mRenderer(renderer), mFontManager(fontManager) {
        FT_Error error = FT_Init_FreeType(&mFTLibrary);
        if (error) {
            std::cerr << "Error: Failed to initialize FreeType library" << std::endl;
        }

        mFontFace = fontManager->getFont();
        if (!mFontFace) {
            std::cerr << "Error: Font not found" << std::endl;
        }
    }

    ~TextRenderer() {
        FT_Done_Face(mFontFace);
        FT_Done_FreeType(mFTLibrary);
    }

    void renderText(const std::string& text, int x, int y, float fontSize, const glm::vec4& color) {
    }

private:
    vkRenderer* mRenderer;
    FontManager* mFontManager;
    FT_Library mFTLibrary;
    FT_Face mFontFace;
};

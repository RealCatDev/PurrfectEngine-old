#include <glm/glm.hpp>
#include <string>
#include <vector>   
#include <unordered_map>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H  

class FontManager {
public:
    FontManager() {
        if (FT_Init_FreeType(&mFTLibrary)) {
            std::cerr << "Error: Failed to initialize FreeType library" << std::endl;
        }
    }

    ~FontManager() {
        FT_Done_FreeType(mFTLibrary);
    }

    bool loadFont(const std::string& fontPath, const std::string& fontName, unsigned int fontSize) {
        FT_Face fontFace;
        if (FT_New_Face(mFTLibrary, fontPath.c_str(), 0, &fontFace)) {
            std::cerr << "Error: Failed to load font from file " << fontPath << std::endl;
            return false;
        }

        if (FT_Set_Char_Size(fontFace, 0, fontSize * 64, 300, 300)) {
            std::cerr << "Error: Failed to set font size for font " << fontName << std::endl;
            FT_Done_Face(fontFace);
            return false;
        }

        mFonts[fontName] = fontFace;
        return true;
    }

    FT_Face getFont(const std::string& fontName) {
        auto it = mFonts.find(fontName);
        if (it != mFonts.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    FT_Library mFTLibrary;
    std::unordered_map<std::string, FT_Face> mFonts;
};

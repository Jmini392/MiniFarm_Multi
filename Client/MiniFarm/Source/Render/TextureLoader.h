#pragma once

struct TextureInfo {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

class TextureLoader
{
public:
    static TextureInfo Load(const std::string& path);

private:
    static std::unordered_map<std::string, TextureInfo> s_cache;

    static TextureInfo LoadInternal(const std::string& path);
};
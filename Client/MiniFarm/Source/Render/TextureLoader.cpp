#include "PCH.h"
#include "TextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unordered_map<std::string, TextureInfo> TextureLoader::s_cache;

TextureInfo TextureLoader::Load(const std::string& path)
{
    auto it = s_cache.find(path);
    if (it != s_cache.end())
        return it->second;

    TextureInfo info = LoadInternal(path);
    if (info.id != 0)
        s_cache[path] = info;

    return info;
}

TextureInfo TextureLoader::LoadInternal(const std::string& path)
{
    TextureInfo info;

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
    {
        LOG_E("Failed to load texture: %s", path.c_str());
        return info;
    }

    GLenum format = (channels == 4) ? GL_RGBA :
        (channels == 3) ? GL_RGB : GL_RED;

    glGenTextures(1, &info.id);
    glBindTexture(GL_TEXTURE_2D, info.id);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
        0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    info.width = width;
    info.height = height;

    stbi_image_free(data);

    return info;
}
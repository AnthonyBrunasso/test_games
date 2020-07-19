#pragma once

struct TextureHandle {
  u32 id = 0;
  Texture texture;
};

struct TextureFileToId {
  u32 id = 0;
};

#define TEXTURE_MAX 64

DECLARE_HASH_ARRAY(TextureHandle, TEXTURE_MAX);
DECLARE_HASH_MAP_STR(TextureFileToId, TEXTURE_MAX);

u32
LoadTexture(const char* file, const TextureInfo& texture_info)
{
  TextureHandle* t = UseTextureHandle();
  if (!LoadTGA(file, texture_info, &t->texture)) {
    printf("Unable to load %s\n", file);
    return 0;
  }
  TextureFileToId* file_to_id = UseTextureFileToId(file, strlen(file));
  file_to_id->id = t->id;
  return t->id; 
}

Texture*
GetTexture(u32 id)
{
  TextureHandle* handle = FindTextureHandle(id);
  if (!handle) return nullptr;
  return &handle->texture;
}

Texture*
GetTexture(const char* file)
{
  TextureFileToId* file_to_id = FindTextureFileToId(file, strlen(file));
  if (!file_to_id) return nullptr;
  return GetTexture(file_to_id->id);
}


void
RenderTexture(u32 id, const Rectf& src, const Rectf& dest, bool mirror = false)
{
  Texture* t = GetTexture(id);
  if (!id) return;
  RenderTexture(*t, src, dest, mirror);
}

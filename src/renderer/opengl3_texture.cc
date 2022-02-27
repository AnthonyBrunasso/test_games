#pragma once

struct Texture {
  char file[64];
  GLuint reference = 0;
  r32 width = 0.f;
  r32 height = 0.f;
  GLenum format;
  Rectf Rect() const {
    return Rectf(0, 0, width, height);
  }

  b8 IsValid() const {
    return width > 0.f && height > 0.f;
  }
};

struct TextureState {
  GLuint vao_reference;
  GLuint vbo_reference;
  GLuint vao_reference_static;
  GLuint program;
  GLuint texture_uniform;
  GLuint matrix_uniform;
  GLuint uv_vbo;
  GLuint frame_buffer = -1;
};

struct TextureInfo {
  GLuint min_filter = GL_LINEAR_MIPMAP_LINEAR;
  GLuint mag_filter = GL_LINEAR;
};

TextureInfo DefaultTextureInfo() {
  TextureInfo info;
  return info;
}

struct UV {
  r32 u;
  r32 v;
};

static TextureState kTextureState;

b8 SetupTexture() {
  GLuint vert_shader, frag_shader;
  if (!GLCompileShader(GL_VERTEX_SHADER, &kTextureVertexShader,
                         &vert_shader)) {
    return false;
  }
  if (!GLCompileShader(GL_FRAGMENT_SHADER, &kTextureFragmentShader,
                         &frag_shader)) {
    return false;
  }
  if (!GLLinkShaders(&kTextureState.program, 2, vert_shader, frag_shader)) {
    return false;
  }
  kTextureState.texture_uniform =
      glGetUniformLocation(kTextureState.program, "basic_texture");
  kTextureState.matrix_uniform =
      glGetUniformLocation(kTextureState.program, "matrix");

  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);

   GLfloat quad[18] = {
    -0.5f,  -0.5f, 0.0f, // BL
    -0.5f,  0.5f, 0.0f, // TL
    0.5f,  0.5f, 0.0f, // TR
    0.5f, -0.5f, 0.0f, // BR
    -0.5f,  -0.5f, 0.0f, // BL
    0.5f,  0.5f, 0.0f, // TR
  };

  kTextureState.vao_reference =
      GLCreateGeometryVAO(18, quad, &kTextureState.vbo_reference);
  u32 vbo;
  kTextureState.vao_reference_static = GLCreateGeometryVAO(18, quad, &vbo);
  glEnableVertexAttribArray(1);
  glGenBuffers(1, &kTextureState.uv_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.uv_vbo);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

  return true;
}

Texture CreateTexture2D(GLenum format, uint64_t width, uint64_t height,
                        TextureInfo texture_info, const void* data) {
  Texture texture = {};
  glGenTextures(1, &texture.reference);
  glBindTexture(GL_TEXTURE_2D, texture.reference);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_info.mag_filter);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_info.min_filter);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    format,
    width,
    height,
    0,
    format,
    GL_UNSIGNED_BYTE,
    data
  );
  if (texture_info.mag_filter == GL_LINEAR_MIPMAP_LINEAR ||
      texture_info.mag_filter == GL_NEAREST_MIPMAP_NEAREST ||
      texture_info.min_filter == GL_LINEAR_MIPMAP_LINEAR ||
      texture_info.min_filter == GL_NEAREST_MIPMAP_NEAREST) {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  texture.width = width;
  texture.height = height;
  texture.format = format;
  return texture;
}

void DestroyTexture2D(Texture* texture) {
  if (texture->reference == 0) return;
  glDeleteTextures(1, &texture->reference);
  *texture = {};
}

Texture CreateEmptyTexture2D(GLenum format, uint64_t width, uint64_t height) {
  Texture texture = {};
  glGenTextures(1, &texture.reference);
  glBindTexture(GL_TEXTURE_2D, texture.reference);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
  texture.width = width;
  texture.height = height;
  texture.format = format;
  return texture;
}

b8 LoadTGA(const char* file, const TextureInfo& texture_info, Texture* texture) {
  // Texture already loaded.
  if (texture->IsValid()) {
    return true;
  }
#pragma pack(push, 1)
  struct TgaImageSpec {
    u16 x_origin;
    u16 y_origin;
    u16 image_width;
    u16 image_height;
    u8 pixel_depth;
    u8 image_descriptor;
  };
  struct TgaHeader {
    u8 id_length;
    u8 color_map_type;
    u8 image_type;
    u8 color_map_spec[5];
  };
#pragma pack(pop)

  FILE* fptr;
  u8* buffer;
  u32 file_length;

  fptr = fopen(file, "rb");
  if (!fptr) {
    printf("Failed  to load %s\n", file);
    return false;
  }
  fseek(fptr, 0, SEEK_END);
  file_length = ftell(fptr);
  rewind(fptr);
  buffer = memory::PushBytes(file_length);
  fread(buffer, file_length, 1, fptr);

  // First load the header.
  TgaHeader* header = (TgaHeader*)buffer;
  // Just don't even support colors.
  assert(header->id_length == 0);
  assert(header->color_map_type == 0);
  // Get the image_spec. This has overall image details.
  TgaImageSpec* image_spec = (TgaImageSpec*)(&buffer[sizeof(TgaHeader)]);

#if 1
  LOG(INFO, "TGA file: %s header", file);
  LOG(INFO, "header->id_length: %i", header->id_length);
  LOG(INFO, "header->color_map_type: %i", header->color_map_type);
  LOG(INFO, "header->image_type: %i", header->image_type); 
  LOG(INFO, "TGA file: %s Image Spec", file);
  LOG(INFO, "image_spec->x_origin: %i", image_spec->x_origin);
  LOG(INFO, "image_spec->y_origin: %i", image_spec->y_origin);
  LOG(INFO, "image_spec->image_width: %i", image_spec->image_width);
  LOG(INFO, "image_spec->image_height: %i", image_spec->image_height);
  LOG(INFO, "image_spec->pixel_depth: %i", image_spec->pixel_depth);
  LOG(INFO, "image_spec->image_descriptor: %i", image_spec->image_descriptor);
#endif

  // Image bytes sz
  u8* image_bytes = &buffer[sizeof(TgaHeader) + sizeof(TgaImageSpec)];
  GLenum format = GL_BGRA;
  if (image_spec->pixel_depth == 8) format = GL_RED;
  else if (image_spec->pixel_depth == 24) format = GL_RGB;
  else if (image_spec->pixel_depth == 32) format = GL_RGBA;
  else {
    LOG(WARN, "Unsupported tga pixel depth\n");
    memory::PopBytes(file_length);
    fclose(fptr);
    return false;
  }
  if (format == GL_RGB || format == GL_RGBA) {
    s32 stride = image_spec->pixel_depth / 8;
    u32 image_bytes_size =
        image_spec->image_width * image_spec->image_height * stride;
    for (s32 i = 0; i < image_bytes_size; i += stride) {
      // Swap bytes for red and blue
      u8 t = image_bytes[i];
      image_bytes[i] = image_bytes[i + 2];
      image_bytes[i + 2] = t;
    }
  }

  *texture = CreateTexture2D(format, image_spec->image_width,
                             image_spec->image_height, texture_info,
                             image_bytes);

  strcpy(texture->file, file);

  // Free buffer used to read in file.
  memory::PopBytes(file_length);
  fclose(fptr);
  return true;
}

void BeginRenderTo(const Texture& texture) {
  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
  if (kTextureState.frame_buffer == GLuint(-1)) {
    glGenFramebuffers(1, &kTextureState.frame_buffer);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, kTextureState.frame_buffer);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.reference, 0);
  GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, draw_buffers);
  glBindFramebuffer(GL_FRAMEBUFFER, kTextureState.frame_buffer);
  glViewport(0, 0, texture.width, texture.height);
}

void EndRenderTo() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  auto dims = window::GetWindowSize();
  glViewport(0, 0, dims.x, dims.y);
}

void RenderTexture(const Texture& texture, const Rectf& src, const Rectf& dest, bool mirror = false) {
  glUseProgram(kTextureState.program);
  glBindTexture(GL_TEXTURE_2D, texture.reference);
  glBindVertexArray(kTextureState.vao_reference_static);
  UV uv[6];
  // Match uv coordinates to quad coords.
  r32 start_x = src.x / texture.width;
  r32 start_y = src.y / texture.height;
  r32 width = src.width / texture.width;
  r32 height = src.height / texture.height;

  if (mirror) {
    uv[0] = {start_x + width, start_y + height}; // BR
    uv[1] = {start_x + width, start_y}; // TR 
    uv[2] = {start_x, start_y}; // TL
    uv[3] = {start_x, start_y + height}; // BL
    uv[4] = {start_x + width, start_y + height}; // BR
    uv[5] = {start_x, start_y}; // TL
  } else {
    uv[0] = {start_x, start_y + height}; // BL
    uv[1] = {start_x, start_y}; // TL
    uv[2] = {start_x + width, start_y}; // TR 
    uv[3] = {start_x + width, start_y + height}; // BR
    uv[4] = {start_x, start_y + height}; // BL
    uv[5] = {start_x + width, start_y}; // TR 
  }
#if 0
  LOG(INFO, "uv[0]=(%.3f, %3.f)", uv[0].u, uv[0].v);
  LOG(INFO, "uv[1]=(%.3f, %3.f)", uv[1].u, uv[1].v);
  LOG(INFO, "uv[2]=(%.3f, %3.f)", uv[2].u, uv[2].v);
  LOG(INFO, "uv[3]=(%.3f, %3.f)", uv[3].u, uv[3].v);
  LOG(INFO, "uv[4]=(%.3f, %3.f)", uv[4].u, uv[4].v);
  LOG(INFO, "uv[5]=(%.3f, %3.f)", uv[5].u, uv[5].v);
#endif
  glBindBuffer(GL_ARRAY_BUFFER, kTextureState.uv_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(uv), uv, GL_DYNAMIC_DRAW);
  v3f pos(dest.x + dest.width / 2.f, dest.y + dest.height / 2.f,0.0f);
  v3f scale(dest.width, dest.height, 1.f);
  Mat4f model = math::Model(pos, scale);
  Mat4f matrix = kObserver.projection * kObserver.view * model;
  glUniformMatrix4fv(kTextureState.matrix_uniform, 1, GL_FALSE, &matrix.data_[0]);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

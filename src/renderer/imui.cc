#pragma once

#include "renderer/renderer.cc"

#include "../common/common.cc"

EXTERN(unsigned imui_errno);

namespace imui
{

enum SpaceType {
  kHorizontal = 0,
  kVertical = 1,
};

enum FlowType {
  kNewLine,
  kSameLine,
};

constexpr s32 kMaxTextSize = 128;
constexpr s32 kClickForFrames = 100;

constexpr r32 kTextScale = 0.8f;
constexpr r32 kScrollBarWidth = 15.f;
constexpr r32 kCheckboxOffset = 2.f;

static const v4f kRed = v4f(1.f, 0.f, 0.f, 1.f);
static const v4f kWhite(1.f, 1.f, 1.f, 1.f);
static const v4f kPaneColor(0.0f, 0.0f, 0.0f, 0.4f);
static const v4f kPaneHeaderColor(0.08f, 0.08f, 0.08f, 1.f);
static const v4f kScrollColor(0.32f, 0.36f, 0.354f, .4f);
static const v4f kScrollHighlightedColor(0.32f, 0.36f, 0.354f, .55f);
static const v4f kScrollSelectedColor(0.32f, 0.36f, 0.354f, .7f);
static const v4f kHeaderMinimizeColor(0.45f, 0.68f, 0.906f, 0.7f);
static const v4f kCheckboxColor(0.32f, 0.36f, 0.54f, 1.0f);
static const v4f kCheckboxCheckedColor(0.32f, 0.36f, 0.954f, 1.f);
static const v4f kCheckboxHighlightedColor(0.32f, 0.36f, 0.954f, .3f);

struct Result {
  Result() = default;
  Result(const Rectf& rect, b8 highlighted, b8 clicked) :
      rect(rect), highlighted(highlighted), clicked(clicked) {}
  Rectf rect;
  b8 highlighted = false;
  b8 clicked = false;
};

#define IMUI_RESULT(rect) \
  Result(rect, IsRectHighlighted(rect), IsRectClicked(rect))

#define IMUI_RESULT_CIRCLE(center, radius) \
  Result(rect, IsCircleHighlighted(center, radius), IsCircleClicked(center, radius))

#define IF_HIDDEN(jmp) \
  if (kIMUI.begin_mode.show && !(*kIMUI.begin_mode.show)) jmp;

#define TITLE_WITH_TAG(title, tag)             \
  char title_with_tag[kMaxHashKeyLength] = {}; \
  char tag_append[2] = {};                     \
  snprintf(tag_append, 2, "%u", tag);          \
  strcat(title_with_tag, title);               \
  strcat(title_with_tag, tag_append);          \

#define DEBUG_POINT(pos) \
  rgg::DebugPushPoint(pos, 3.5f, v4f(0.f, .5f, 0.5f, 1.f), rgg::kDebugUI);

#define DEBUG_RECT(rect) \
  rgg::DebugPushRect(rect, v4f(0.f, 0.f, 1.f, 1.f), rgg::kDebugUI);

struct TextOptions {
  v4f color = kWhite;
  v4f highlight_color = v4f();
  // If true the text will render regardless of clipping options.
  b8 ignore_scissor_test = false;
};

struct ButtonCircleOptions {
  b8 ignore_scissor_test = false;
};

struct PaneOptions {
  // Starting with and height of pane.
  r32 width = 0.f;
  r32 height = 0.f;
  // If set the pane will not expand these dimensions.
  r32 max_width = 0.f;
  r32 max_height = 0.f;
  v4f color = kPaneColor;
  b8 enable_console_mode = false;
  Rectf header_rect;
};

// imui elements.

enum PaneFlag {
  // Set if the pane exists but should be minimized.
  kPaneHidden,
  // If set will render information about the pane via imui::DebugPane.
  kPaneDebug,
  // Set if the pane has a scroll bar.
  kPaneHasScroll,
  // Set if the user is scrolling the pane with their mouse.
  kPaneIsScrolling,
  // Used to cleanup the pane when the user does not call Begin() on the pane.
  kPaneActive,
  // Console mode causes the pane to clamp to it's max scroll while it's at
  // max scroll but will allow scrolling up to see history.
  kPaneConsoleMode,
};

struct Pane {
  u32 tag;
  u32 flags;
  Rectf rect;
  PaneOptions options;
  // Now far up or down the user has scrolled.
  // 0 indicates no scroll.
  // A value greater than 0 means the user has scrolled down. All text ui
  // elements should be positioned accordingly.
  r32 vertical_scroll = 0.f;
  // Current max scroll can be calculated as theoretical_height - rect.height.
  // This value is the max scroll allowed from the previous call to Begin.
  r32 previous_max_scroll = 0.f;
  b8 element_off_pane = false;
  Rectf header_rect;
  // The height the pane would be if constraints like max_height screen
  // clipping where done. This is used for scrolling.
  r32 theoretical_height = FLT_MIN;
  r32 theoretical_y = FLT_MAX;
  Rectf scroll_rect;
  char title[kMaxHashKeyLength];
  // Docked panes as linked lists with forward and backward pane pointers.
  Pane* next_pane = nullptr;
  Pane* prev_pane = nullptr;
};

struct Text {
  char msg[kMaxTextSize];
  v2f pos;
  v4f color;  // TODO: This is duplicated in TextOptions
  Rectf rect;
  TextOptions options;
  Pane* pane;
};

struct Button {
  Rectf rect;
  v4f color;
  Pane* pane;
};

struct ButtonCircle {
  v2f position;
  r32 radius;
  v4f color;
  ButtonCircleOptions options;
  Pane* pane;
};

struct Line {
  v2f start;
  SpaceType type;
  v4f color;
  Pane* pane;
};

struct ProgressBar {
  Rectf rect;
  r32 current_progress;
  r32 max_progress;
  v4f fill_color;
  v4f outline_color;
  Pane* pane;
};

struct Checkbox {
  Rectf rect;
  b8 checked;
  b8 is_highlighted;
  Pane* pane;
};

struct Texture {
  Rectf rect;
  Rectf subrect;
  u32 texture_id;
  Pane* pane;
};

// imui metadata.

struct MouseDown {
  v2f pos;
  PlatformButton button;
};

struct MouseUp {
  v2f pos;
  PlatformButton button;
};

struct MouseWheel {
  r32 delta;
};

struct MousePosition {
  v2f pos;
};

struct LastMousePosition {
  v2f pos;
};

struct BeginMode {
  v2f pos;
  b8 set = false;
  u32 tag = 0;
  // UI Element go one new line unless explcitly swapped.
  FlowType flow_type = kNewLine;
  b8 flow_switch = false;
  Rectf last_rect;
  b8 mouse_down;
  // A bit of a hack?
  b8 ignore_vertical_scroll = false;
  r32 overwrite_width;
  u32 indent = 0;
  b8* show = nullptr;
  v2f* start = nullptr;
  Pane* pane;
};

#ifdef SINGLE_PLAYER
constexpr u32 kMaxTags = 1;
constexpr u32 kEveryoneTag = 0;
#else
constexpr u32 kMaxTags = MAX_PLAYER + 1;
constexpr u32 kEveryoneTag = MAX_PLAYER;
#endif

struct IMUI {
  BeginMode begin_mode;
  b8 mouse_down[kMaxTags];
  u32 text_exhaustion[kMaxTags];
  u32 button_exhaustion[kMaxTags];
  u32 button_circle_exhaustion[kMaxTags];
  b8 debug_show_details[kMaxTags];
  b8 debug_enabled = false;
};

static IMUI kIMUI;

DECLARE_2D_ARRAY(Text, kMaxTags, 128);
DECLARE_2D_ARRAY(Line, kMaxTags, 32);
DECLARE_2D_ARRAY(Button, kMaxTags, 32);
DECLARE_2D_ARRAY(ButtonCircle, kMaxTags, 16);
DECLARE_2D_ARRAY(Texture, kMaxTags, 32);
DECLARE_2D_ARRAY(Checkbox, kMaxTags, 16);
DECLARE_2D_ARRAY(MouseDown, kMaxTags, 8);
DECLARE_2D_ARRAY(MouseUp, kMaxTags, 8);
DECLARE_2D_ARRAY(MouseWheel, kMaxTags, 8);
DECLARE_2D_ARRAY(ProgressBar, kMaxTags, 16);
DECLARE_2D_ARRAY(MousePosition, kMaxTags, 4);
DECLARE_2D_ARRAY(LastMousePosition, kMaxTags, 4);
// Panes exist as a global UI element that persist per imui begin / end calls.
// This allows for panes to stick around for bounds check, docking and imui
// debugability.
DECLARE_HASH_MAP_STR(Pane, 64);

void
ToggleDebug()
{
  kIMUI.debug_enabled = !kIMUI.debug_enabled;
}

void
GenerateUIMetadata(u32 tag)
{
  memcpy(kLastMousePosition[tag], kMousePosition[tag],
         sizeof(kMousePosition[tag]));
  kUsedLastMousePosition[tag] = kUsedMousePosition[tag];
}

void
ResetAll()
{
  for (int i = 0; i < kMaxTags; ++i) {
    GenerateUIMetadata(i);
  }
  memset(kUsedText, 0, sizeof(kUsedText));
  memset(kUsedButton, 0, sizeof(kUsedButton));
  memset(kUsedButtonCircle, 0, sizeof(kUsedButton));
  memset(kUsedTexture, 0, sizeof(kUsedTexture));
  memset(kUsedCheckbox, 0, sizeof(kUsedCheckbox));
  memset(kUsedMouseDown, 0, sizeof(kUsedMouseDown));
  memset(kUsedMouseUp, 0, sizeof(kUsedMouseUp));
  memset(kUsedMousePosition, 0, sizeof(kUsedMousePosition));
  memset(kUsedMouseWheel, 0, sizeof(kUsedMouseWheel));
  memset(kUsedProgressBar, 0, sizeof(kUsedProgressBar));
  memset(kUsedLine, 0, sizeof(kUsedLine));
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    CBIT(pane->flags, kPaneActive);
  }
}

void
ResetTag(u32 tag)
{
  assert(tag < kMaxTags);
  GenerateUIMetadata(tag);
  kUsedText[tag] = 0;
  kUsedButton[tag] = 0;
  kUsedButtonCircle[tag] = 0;
  kUsedTexture[tag] = 0;
  kUsedCheckbox[tag] = 0;
  kUsedMouseDown[tag] = 0;
  kUsedMouseUp[tag] = 0;
  kUsedMousePosition[tag] = 0;
  kUsedMouseWheel[tag] = 0;
  kUsedProgressBar[tag] = 0;
  kUsedLine[tag] = 0;
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    CBIT(pane->flags, kPaneActive);
  }
}

v2f
MouseDelta(u32 tag)
{
  if (kUsedMousePosition[tag] < 1 || kUsedLastMousePosition[tag] < 1)
    return {};
  return kMousePosition[tag][0].pos - kLastMousePosition[tag][0].pos;
}

v2f
MouseDelta()
{
  return MouseDelta(kIMUI.begin_mode.tag);
}

void
SetScissorWithPane(const Pane& pane, const v2f& viewport, b8 ignore_scissor)
{
  if (ignore_scissor) {
    glScissor(0, 0, viewport.x, viewport.y);
  } else {
    // Scissor from header down.
    glScissor(
        pane.rect.x, pane.rect.y,
        pane.rect.width - (FLAGGED(pane.flags, kPaneHasScroll) ?
            pane.scroll_rect.width : 0.f),
        pane.rect.height - pane.header_rect.height);
  }
}

b8
IsRectHighlighted(Rectf rect)
{
  u32 tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (math::PointInRect(mp->pos, rect)) return true;
  }
  return false;
}

void
Render(u32 tag)
{
  glDisable(GL_DEPTH_TEST);
  auto dims = window::GetWindowSize();
  // printf("dims(%.2f, %.2f)\n", dims.x, dims.y);
  rgg::ModifyObserver mod(math::Ortho2(dims.x, 0.0f, dims.y, 0.0f, 0.0f, 0.0f),
                          math::Identity());

  // Free inactive panes - this is a pretty rare event. Ok to perform in
  // Render step.
  for (int i = 0; i < kUsedPane;) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) { ++i; continue; }
    if (!FLAGGED(pane->flags, kPaneActive)) {
      TITLE_WITH_TAG(pane->title, tag);
      ErasePane(title_with_tag, strlen(title_with_tag));
      continue;
    }
    ++i;
  }

  kIMUI.text_exhaustion[tag] = kUsedText[tag];
  kIMUI.button_exhaustion[tag] = kUsedButton[tag];
  kIMUI.button_circle_exhaustion[tag] = kUsedButtonCircle[tag];

  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    rgg::RenderRectangle(pane->rect, pane->options.color);
    rgg::RenderRectangle(pane->header_rect, kPaneHeaderColor);
    rgg::RenderLineRectangle(
        pane->rect, v4f(0.2f, 0.2f, 0.2f, 0.7f));
    if (FLAGGED(pane->flags, kPaneHasScroll) &&
        !FLAGGED(pane->flags, kPaneHidden)) {
      if (FLAGGED(pane->flags, kPaneIsScrolling)) {
        rgg::RenderRectangle(pane->scroll_rect, kScrollSelectedColor);
      } else if (IsRectHighlighted(pane->scroll_rect)) {
        rgg::RenderRectangle(pane->scroll_rect, kScrollHighlightedColor);
      } else {
        rgg::RenderRectangle(pane->scroll_rect, kScrollColor);
      }
    }
  }

  for (int i = 0; i < kUsedButton[tag]; ++i) {
    Button* button = &kButton[tag][i];
    //SetScissorWithPane(*button->pane, dims, false);
    rgg::RenderLineRectangle(button->rect, button->color);
  }

  for (int i = 0; i < kUsedButtonCircle[tag]; ++i) {
    ButtonCircle* button = &kButtonCircle[tag][i];
    //SetScissorWithPane(*button->pane, dims,
    //                   button->options.ignore_scissor_test);
    rgg::RenderCircle(button->position, button->radius, button->color);
  }

  for (int i = 0; i < kUsedTexture[tag]; ++i) {
    Texture* texture = &kTexture[tag][i];
    //SetScissorWithPane(*texture->pane, dims, false);
    rgg::RenderTexture(texture->texture_id, texture->subrect, texture->rect);
  }

  for (int i = 0; i < kUsedCheckbox[tag]; ++i) {
    Checkbox* cb = &kCheckbox[tag][i];
    //SetScissorWithPane(*cb->pane, dims, false);
    rgg::RenderLineRectangle(cb->rect, kCheckboxColor);
    if (cb->checked) {
      Rectf crect(cb->rect);
      crect.width /= 1.25f;
      crect.height /= 1.25f;
      crect.x  += (cb->rect.width - crect.width) / 2.f;
      crect.y  += (cb->rect.height - crect.height) / 2.f;
      rgg::RenderRectangle(crect, kCheckboxCheckedColor);
    } else if (cb->is_highlighted) {
      Rectf crect(cb->rect);
      crect.width /= 1.25f;
      crect.height /= 1.25f;
      crect.x  += (cb->rect.width - crect.width) / 2.f;
      crect.y  += (cb->rect.height - crect.height) / 2.f;
      rgg::RenderRectangle(crect, kCheckboxHighlightedColor);
    }
  }

  for (int i = 0; i < kUsedText[tag]; ++i) {
    Text* text = &kText[tag][i];
    //SetScissorWithPane(*text->pane, dims, text->options.ignore_scissor_test);
    rgg::RenderText(text->msg, text->pos, kTextScale, text->color);
  }

  for (int i = 0; i < kUsedLine[tag]; ++i) {
    Line* line = &kLine[tag][i];
    //SetScissorWithPane(*line->pane, dims, false);
    if (line->type == kHorizontal) {
      v2f end(line->start.x + line->pane->rect.width, line->start.y);
      rgg::RenderLine(line->start, end, line->color);
    }
  }

  for (int i = 0; i < kUsedProgressBar[tag]; ++i) {
    ProgressBar* pb = &kProgressBar[tag][i];
    //SetScissorWithPane(*pb->pane, dims, false);
    rgg::RenderProgressBar(
        pb->rect, 0.f, pb->current_progress, pb->max_progress, pb->fill_color,
        pb->outline_color);
  }
  //glScissor(0, 0, dims.x, dims.y);
  glEnable(GL_DEPTH_TEST);
}

r32
GetMouseWheel()
{
  u32 tag = kIMUI.begin_mode.tag;
  if (!kUsedMouseWheel[tag]) return 0.f;
  r32 d = 0.f;
  for (int i = 0; i < kUsedMouseWheel[tag]; ++i) {
    d += kMouseWheel[tag][i].delta;
  }
  return d;
}

v2f
GetMousePosition()
{
  u32 tag = kIMUI.begin_mode.tag;
  if (!kUsedMousePosition[tag]) return {};
  return kMousePosition[tag][0].pos;
}

b8
IsRectPreviouslyHighlighted(Rectf rect)
{
  u32 tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedLastMousePosition[tag]; ++i) {
    LastMousePosition* mp = &kLastMousePosition[tag][i];
    if (math::PointInRect(mp->pos, rect)) return true;
  }
  return false;
}

b8
IsRectClicked(Rectf rect)
{
  u32 tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMouseDown[tag]; ++i) {
    MouseDown* click = &kMouseDown[tag][i];
    if (math::PointInRect(click->pos, rect)) return true;
  }
  return false;
}

b8
IsCircleHighlighted(v2f center, r32 radius)
{
  u32 tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (math::PointInCircle(mp->pos, center, radius)) return true;
  }
  return false;
}

b8
IsCircleClicked(v2f center, r32 radius)
{
  u32 tag = kIMUI.begin_mode.tag;
  for (int i = 0; i < kUsedMouseDown[tag]; ++i) {
    MouseDown* click = &kMouseDown[tag][i];
    if (math::PointInCircle(click->pos, center, radius)) return true;
  }
  return false;
}

b8
IsMouseDown(u32 tag)
{
  return kIMUI.mouse_down[tag];
}

b8
IsMouseDown()
{
  return IsMouseDown(kIMUI.begin_mode.tag);
}

u32
WidthOf(char c)
{
  auto& font = rgg::kUI.font;
  rgg::FontMetadataRow* row = &rgg::kFontMetadataRow[' '];
  if (!row || !row->id) return 0;
  return row->xadvance;
}

void
Indent(int spaces)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  auto& font = rgg::kUI.font;
  rgg::FontMetadataRow* row = &rgg::kFontMetadataRow[' '];
  if (!row || !row->id) return;
  kIMUI.begin_mode.indent = spaces;
  kIMUI.begin_mode.pos.x = kIMUI.begin_mode.start->x + spaces * row->xadvance;
}

// Returns rect representing bounds for the current object.
// Updates global bounds of pane.
// Set begin_mode.pos to point to the bottom left of where the current element
// should draw.
Rectf
UpdatePane(r32 width, r32 height, b8* element_in_pane)
{
  assert(element_in_pane);
  IF_HIDDEN(return Rectf());
  auto& begin_mode = kIMUI.begin_mode;
  assert(begin_mode.set);
  assert(begin_mode.pane);
  width = begin_mode.overwrite_width > 0.f ?
      begin_mode.overwrite_width : width;
  begin_mode.overwrite_width = 0.f;
  // Determine if element goes on same line or new line.
  bool new_line = begin_mode.flow_type == kNewLine || begin_mode.flow_switch;
  begin_mode.flow_switch = false;
  // Move begin_mode.pos to where the next element should be.
  if (new_line) {
    begin_mode.pos.y -= height; 
  } else {
    begin_mode.pos.x += begin_mode.last_rect.width;
  }
  // Determine the new pane size.
  Pane* pane = begin_mode.pane;
  if (pane->options.width) {
    pane->rect.width = pane->options.width;
  } else {
    // The new width is the growth in x if the elements position outgrows
    // the width of the panel.
    float max_x = begin_mode.pos.x + width;
    if (max_x > pane->rect.Max().x) {
      pane->rect.width += max_x - pane->rect.Max().x;
    }
    if (pane->options.max_width > 0.f &&
        pane->rect.width >= pane->options.max_width) {
      pane->rect.width = pane->options.max_width;
      pane->rect.x = begin_mode.start->x + pane->rect.width;
    }
  }
  if (pane->options.height) {
    pane->rect.height = pane->options.height;
    pane->theoretical_height = pane->rect.height;
  } else {
    pane->rect.height += pane->rect.y - begin_mode.pos.y;
    pane->theoretical_height = begin_mode.start->y - begin_mode.pos.y;
    pane->rect.y = begin_mode.pos.y;
    // The height has surpssed the max allowed height - correct it.
    if (pane->options.max_height > 0.f &&
        pane->rect.height >= pane->options.max_height) {
      pane->rect.height = pane->options.max_height;
      pane->rect.y = begin_mode.start->y - pane->rect.height;
    }
  }
  begin_mode.last_rect = Rectf(begin_mode.pos, width, height);
  if (!begin_mode.ignore_vertical_scroll) {
    begin_mode.last_rect.y += begin_mode.pane->vertical_scroll;
  }
  pane->theoretical_y = math::Min(begin_mode.pos.y, pane->theoretical_y);
  *element_in_pane =
      math::IntersectRect(begin_mode.last_rect, pane->rect);
  begin_mode.pane->element_off_pane =
      !math::IsContainedInRect(begin_mode.last_rect, pane->rect);
  if (FLAGGED(begin_mode.pane->flags, kPaneDebug) && *element_in_pane) {
    DEBUG_POINT(v2f(begin_mode.last_rect.x, begin_mode.last_rect.y));
    DEBUG_RECT(begin_mode.last_rect);
  }
  return begin_mode.last_rect;
}

Result
Text(const char* msg, TextOptions options)
{
  assert(kIMUI.begin_mode.set);
  auto& begin_mode = kIMUI.begin_mode;
  u32 tag = kIMUI.begin_mode.tag;
  Result data;
  IF_HIDDEN(return data);
  if (strlen(msg) > kMaxTextSize) {
    imui_errno = 2;
    return data;
  }
  Rectf text_rect = 
      rgg::GetTextRect(msg, strlen(msg), begin_mode.pos, kTextScale);
  b8 in_pane = false;
  Rectf rect = UpdatePane(text_rect.width, text_rect.height, &in_pane);
  if (!in_pane) return data;
  struct Text* text = UseText(tag);
  if (!text) {
    imui_errno = 1;
    return data;
  }
  strcpy(text->msg, msg);
  text->pos = v2f(rect.x, rect.y);
  text->color = options.color;
  if (IsRectHighlighted(rect) && options.highlight_color != v4f()) {
    text->color = options.highlight_color;
  }
  text->options = options;
  text->rect = rect;
  text->pane = begin_mode.pane;
  return IMUI_RESULT(rect);
}

Result
Text(const char* msg)
{
  assert(kIMUI.begin_mode.set);
  return Text(msg, {kWhite, kWhite});
}

Result
Bitfield32(u32 bitfield)
{
  char bstr[33];
  for (int i = 31; i > -1; --i) {
    if (FLAGGED(bitfield, 31 - i)) bstr[i] = '1';
    else bstr[i] = '0';
  }
  bstr[32] = '\0';
  return Text(bstr);
}

Result
Bitfield16(uint8_t bitfield)
{
  char bstr[17];
  for (int i = 15; i > -1; --i) {
    if (FLAGGED(bitfield, 15 - i)) bstr[i] = '1';
    else bstr[i] = '0';
  }
  bstr[16] = '\0';
  return Text(bstr);
}

Result
Bitfield8(uint8_t bitfield)
{
  char bstr[9];
  for (int i = 7; i > -1; --i) {
    if (FLAGGED(bitfield, 7 - i)) bstr[i] = '1';
    else bstr[i] = '0';
  }
  bstr[8] = '\0';
  return Text(bstr);
}

void
HorizontalLine(const v4f& color)
{
  assert(kIMUI.begin_mode.set);
  u32 tag = kIMUI.begin_mode.tag;
  IF_HIDDEN(return);
  b8 in_pane = false;
  Rectf rect = UpdatePane(kIMUI.begin_mode.pane->rect.width, 1.f, &in_pane);
  // TODO(abrunass): Not working for horizontal lines - not sure why.
  //if (!in_pane) return;
  Line* line = UseLine(tag);
  if (!line) {
    imui_errno = 5;
    return;
  }
  line->start.x = rect.x;
  line->start.y = rect.y;
  line->pane = kIMUI.begin_mode.pane;
  line->color = color;
  line->pane = kIMUI.begin_mode.pane;
}

void
Space(SpaceType type, int count)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  b8 in_pane = false;
  if (type == kHorizontal) {
    UpdatePane(count, 0.f, &in_pane);
    if (!in_pane) return;
  } else {
    UpdatePane(0.f, count, &in_pane);
    if (!in_pane) return;
  }
}

Result
Button(r32 width, r32 height, const v4f& color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  u32 tag = kIMUI.begin_mode.tag;
  Result result;
  IF_HIDDEN(return result);
  b8 in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct Button* button = UseButton(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  button->rect = rect;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(button->rect);
}

Result
Texture(r32 width, r32 height, u32 texture_id, Rectf subrect)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  u32 tag = kIMUI.begin_mode.tag;
  Result result;
  IF_HIDDEN(return result);
  b8 in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct Texture* texture = UseTexture(tag);
  if (!texture) {
    imui_errno = 3;
    return result;
  }
  texture->texture_id = texture_id;
  texture->rect = rect;
  texture->subrect = subrect;
  texture->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(texture->rect);
}

Result
ButtonCircle(r32 radius, const v4f& color,
             const ButtonCircleOptions& options)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  Result result;
  IF_HIDDEN(return result);
  u32 tag = kIMUI.begin_mode.tag;
  b8 in_pane = false;
  Rectf rect = UpdatePane(2.f * radius, 2.f * radius, &in_pane);
  if (!in_pane) return result;
  struct ButtonCircle* button = UseButtonCircle(tag);
  if (!button) {
    imui_errno = 3;
    return result;
  }
  // RenderButton renders from center.
  button->position = v2f(rect.x, rect.y) + v2f(radius, radius);
  button->radius = radius;
  button->color = color;
  button->pane = kIMUI.begin_mode.pane;
  button->options = options;
  return IMUI_RESULT_CIRCLE(button->position, radius);
}

Result
ButtonCircle(r32 radius, const v4f& color)
{
  ButtonCircleOptions options;
  return ButtonCircle(radius, color, options);
}

Result
Checkbox(r32 width, r32 height, b8* checked)
{
  assert(kIMUI.begin_mode.set);
  Result result;
  IF_HIDDEN(return result);
  u32 tag = kIMUI.begin_mode.tag;
  b8 in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct Checkbox* checkbox = UseCheckbox(tag);
  if (!checkbox) {
    imui_errno = 3;
    return result;
  }
  checkbox->rect = rect;
  if (IsRectClicked(rect)) (*checked) = !(*checked);
  checkbox->checked = *checked;
  checkbox->is_highlighted = IsRectHighlighted(rect);
  checkbox->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(checkbox->rect);
}

Result
ProgressBar(r32 width, r32 height, r32 current_progress,
            r32 max_progress, const v4f& fill_color,
            const v4f& outline_color)
{
  // Call Begin() before imui elements.
  assert(kIMUI.begin_mode.set);
  u32 tag = kIMUI.begin_mode.tag;
  Result result;
  IF_HIDDEN(return result);
  b8 in_pane = false;
  Rectf rect = UpdatePane(width, height, &in_pane);
  if (!in_pane) return result;
  struct ProgressBar* pb = UseProgressBar(tag);
  if (!pb) {
    imui_errno = 3;
    return result;
  }
  pb->rect = rect;
  pb->current_progress = current_progress;
  pb->max_progress = max_progress;
  pb->outline_color = outline_color;
  pb->fill_color = fill_color;
  pb->pane = kIMUI.begin_mode.pane;
  return IMUI_RESULT(pb->rect);
}

void
SameLine()
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.flow_type = kSameLine;
  kIMUI.begin_mode.flow_switch = true;
}

// Overwrite width to a certain value for the next imui call.
void
Width(r32 width)
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  kIMUI.begin_mode.overwrite_width = width;
}

void
NewLine()
{
  assert(kIMUI.begin_mode.set);
  IF_HIDDEN(return);
  BeginMode* begin_mode = &kIMUI.begin_mode;
  begin_mode->flow_type = kNewLine;
  begin_mode->flow_switch = true;
  auto& font = rgg::kUI.font;
  rgg::FontMetadataRow* row = &rgg::kFontMetadataRow[' '];
  if (!row || !row->id) return;
  begin_mode->pos.x = begin_mode->start->x + begin_mode->indent * row->xadvance;
}

void
Begin(const char* title, u32 tag, const PaneOptions& pane_options, v2f* start,
      b8* show = nullptr)
{
  assert(tag < kMaxTags);
  assert(title);
  if (pane_options.max_width)
    assert(pane_options.width <= pane_options.max_width);
  if (pane_options.max_height)
    assert(pane_options.height <= pane_options.max_height);
  TITLE_WITH_TAG(title, tag);
  u32 title_with_tag_len = strlen(title_with_tag);
  assert(title_with_tag_len < kMaxHashKeyLength);
  auto& begin_mode = kIMUI.begin_mode;
  // End must be called before Begin.
  assert(!begin_mode.set);
  begin_mode.pos = *start;
  begin_mode.set = true;
  begin_mode.tag = tag;
  begin_mode.start = start;
  begin_mode.pane = FindOrUsePane(title_with_tag, title_with_tag_len);
  u32 title_len = strlen(begin_mode.pane->title);
  strcpy(begin_mode.pane->title, title);
  // Header. TODO(abrunasso): imui now relies on hashing header title for pane
  // persistence - but it is worth adding a pane option to hide it here.
  SameLine();
  begin_mode.pos.x += 5.f;
  Rectf t = rgg::GetTextRect(title, title_len, *start, kTextScale);
  begin_mode.pane->tag = tag;
  begin_mode.pane->rect.width =
      pane_options.width > 0.f ? pane_options.width : t.width;
  begin_mode.pane->rect.height =
      pane_options.height > 0.f ? pane_options.height : 0.f;
  begin_mode.pane->theoretical_height = 0.f;
  begin_mode.pane->rect.x = start->x;
  begin_mode.pane->rect.y = start->y - begin_mode.pane->rect.height;
  begin_mode.pane->options = pane_options;
  begin_mode.pane->theoretical_y = start->y;
  if (pane_options.enable_console_mode) {
    SBIT(begin_mode.pane->flags, kPaneConsoleMode);
  } else CBIT(begin_mode.pane->flags, kPaneConsoleMode);
  if (show && !(*show)) SBIT(begin_mode.pane->flags, kPaneHidden);
  else CBIT(begin_mode.pane->flags, kPaneHidden);
  SBIT(begin_mode.pane->flags, kPaneActive);
  // Header is not effect by vertical scroll - it's kinda special.
  begin_mode.ignore_vertical_scroll = true;
  ButtonCircleOptions button_options;
  button_options.ignore_scissor_test = true;
  if (ButtonCircle(10.f, kHeaderMinimizeColor, button_options).clicked) {
    if (show) (*show) = !(*show);
  }
  begin_mode.pos.y -= 2.f;
  begin_mode.pos.x += 5.f;
  TextOptions text_options; 
  text_options.ignore_scissor_test = true;
  Rectf trect = Text(title, text_options).rect;
  begin_mode.ignore_vertical_scroll = false;
  begin_mode.pane->header_rect.height = trect.height;
  NewLine();
  if (show) begin_mode.show = show;
}

void
Begin(const char* title, u32 tag, v2f* start, b8* show = nullptr)
{
  PaneOptions pane_options;
  pane_options.color = v4f(0.f, 0.f, 0.f, 0.f);
  Begin(title, tag, pane_options, start, show);
}

b8
CanScroll(const Pane& pane, r32 delta)
{
  if (delta > 0.f && pane.element_off_pane) return true;
  if (delta < 0.f && pane.vertical_scroll > 0.f) return true;
  return false;
}

r32
GetMaxScroll(const Pane& pane)
{
  // HACK: - .1f to allow scrolling up once the bar has reached the bottom.
  return (pane.theoretical_height - pane.rect.height) - .1f;
}

void
ClampVerticalScroll()
{
  assert(kIMUI.begin_mode.set);
  Pane* pane = kIMUI.begin_mode.pane;
  // Clamp the vertical scroll to the min and max possible scroll.
  if (pane->vertical_scroll < 0.f) pane->vertical_scroll = 0.f;
  if (pane->rect.height + pane->vertical_scroll > pane->theoretical_height) {
    pane->vertical_scroll = GetMaxScroll(*pane);
  }
}

void
End()
{
  assert(kIMUI.begin_mode.set);
  Pane* pane = kIMUI.begin_mode.pane;
  pane->header_rect.x = kIMUI.begin_mode.start->x;
  pane->header_rect.y = kIMUI.begin_mode.start->y - pane->header_rect.height;
  pane->header_rect.width = pane->rect.width;
  if (FLAGGED(pane->flags, kPaneHidden)) pane->rect = pane->header_rect;
  // Move around panes if a user has click and held in them.
  if (kIMUI.begin_mode.start && IsMouseDown() &&
      IsRectPreviouslyHighlighted(kIMUI.begin_mode.pane->rect) &&
      !FLAGGED(pane->flags, kPaneIsScrolling)) {
    *kIMUI.begin_mode.start += MouseDelta();
  }
  r32 d = GetMouseWheel();
  b8 pane_highlighted = IsRectHighlighted(pane->rect);
  r32 original_scroll = pane->vertical_scroll;
  b8 original_scroll_at_max =
      pane->vertical_scroll >= pane->previous_max_scroll;
  if (CanScroll(*pane, d) && pane_highlighted && d != 0.f) {
    pane->vertical_scroll += d;
  }
  ClampVerticalScroll();
  CBIT(pane->flags, kPaneHasScroll);
  if (pane->theoretical_height > pane->rect.height) {
    SBIT(pane->flags, kPaneHasScroll);
    Rectf scroll_bar(
        pane->rect.x + pane->rect.width - kScrollBarWidth, pane->rect.y,
        kScrollBarWidth, pane->rect.height - pane->header_rect.height);
    r32 hdiff = pane->theoretical_height - pane->rect.height;
    r32 p_off = pane->rect.height / pane->theoretical_height;
    r32 scroll_rect_height =
        (pane->rect.height - pane->header_rect.height) * p_off;
    Rectf scroll_cursor(scroll_bar);
    scroll_cursor.height = scroll_rect_height;
    scroll_cursor.y += (scroll_bar.height - scroll_rect_height);
    r32 p_diff = pane->vertical_scroll /
        (pane->theoretical_height - pane->rect.height);
    r32 p_bar_diff_to_bot = fabs(scroll_cursor.y) - fabs(pane->rect.y);
    scroll_cursor.y -= (p_bar_diff_to_bot * p_diff);
    pane->scroll_rect = scroll_cursor;
    if (FLAGGED(pane->flags, kPaneIsScrolling)) {
      r32 bar_to_bot = pane->scroll_rect.y - pane->rect.y;
      r32 vscroll_to_bot =
          pane->theoretical_height - pane->rect.height - pane->vertical_scroll;
      if (vscroll_to_bot > FLT_EPSILON) {
        pane->vertical_scroll -= math::ScaleRange(
            MouseDelta().y, bar_to_bot, vscroll_to_bot);
      }
    }
  }
  if (FLAGGED(pane->flags, kPaneConsoleMode) &&
      original_scroll_at_max && original_scroll <= pane->vertical_scroll) {
    pane->vertical_scroll = GetMaxScroll(*pane);
  }
  pane->previous_max_scroll = GetMaxScroll(*pane);
  ClampVerticalScroll();
  kIMUI.begin_mode = {};
}

void
DockWith(const char* title)
{
  if (!title) return;
  assert(kIMUI.begin_mode.set);
  auto& begin_mode = kIMUI.begin_mode;
  u32 tag = kIMUI.begin_mode.tag;
  TITLE_WITH_TAG(title, tag);
  Pane* pane = FindPane(title_with_tag, strlen(title_with_tag));
  if (!pane) return;
  pane->next_pane = kIMUI.begin_mode.pane;
  begin_mode.pane->prev_pane = pane->next_pane;
  SBIT(begin_mode.pane->flags, kPaneHidden);
  begin_mode.pane->rect = pane->rect;
  (*begin_mode.start) = pane->rect.Min() + v2f(0.f, pane->rect.height);
  (*begin_mode.show) = false;
}

void
VerticalScrollDelta(r32 delta)
{
  assert(kIMUI.begin_mode.set);
  kIMUI.begin_mode.pane->vertical_scroll += delta;
  if (kIMUI.begin_mode.pane->vertical_scroll < 0.f) {
    kIMUI.begin_mode.pane->vertical_scroll = 0.f;
  }
}

b8
MouseInUI(v2f pos, u32 tag)
{
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    if (math::PointInRect(pos, pane->rect)) return true;
  }
  return false;
}

b8
MouseInUI(u32 tag)
{
  for (int i = 0; i < kUsedMousePosition[tag]; ++i) {
    MousePosition* mp = &kMousePosition[tag][i];
    if (MouseInUI(mp->pos, tag)) return true;
  }
  return false;
}

void
MouseDown(v2f pos, PlatformButton b, u32 tag)
{
  // Only record mouse down events if they occurred in the UI.
  if (!MouseInUI(pos, tag)) return;
  struct MouseDown* click = UseMouseDown(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
  kIMUI.mouse_down[tag] = true;
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    if (!FLAGGED(pane->flags, kPaneHasScroll)) continue;
    if (math::PointInRect(pos, pane->scroll_rect)) {
      SBIT(pane->flags, kPaneIsScrolling);
      break;
    }
  }
}

void
MouseUp(v2f pos, PlatformButton b, u32 tag)
{
  struct MouseUp* click = UseMouseUp(tag);
  if (!click) {
    imui_errno = 4;
    return;
  }
  click->pos = pos;
  click->button = b;
  kIMUI.mouse_down[tag] = false;
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    if (pane->tag != tag) continue;
    if (!FLAGGED(pane->flags, kPaneHasScroll)) continue;
    CBIT(pane->flags, kPaneIsScrolling);
  }
}

void
MouseWheel(r32 delta, u32 tag)
{
  struct MouseWheel* wheel = UseMouseWheel(tag);
  if (!wheel) {
    imui_errno = 4;
    return;
  }
  wheel->delta = -delta * 5.f;
}

// Returns true if the mouse is contained within UI given bounds of last
// UI frame.
b8
MousePosition(v2f pos, u32 tag)
{
  struct MousePosition* mp = UseMousePosition(tag);
  if (!mp) {
    imui_errno = 4;
    return false;
  }
  mp->pos = pos;
  return MouseInUI(pos, tag);
}

// Prints useful imui internals for debugging.
void
DebugPane(const char* title, u32 tag, v2f* pos, b8* show)
{
  char buffer[64];
  PaneOptions pane_options;
  pane_options.max_height = 600.f;
  Begin(title, tag, pane_options, pos, show);
  snprintf(buffer, 64, "Pane Count (%u) Max Hash Count (%u) Collision (%.2f%%)",
           kUsedPane, kMaxHashPane,
           ((r32)(kFindCollisionsPane) / kFindCallsPane) * 100.f);
  Text(buffer);
  Text("Panes");
  HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  for (int i = 0; i < kUsedPane; ++i) {
    Pane* pane = &kPane[i];
    TextOptions toptions;
    toptions.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
    if (Text(pane->title, toptions).clicked) {
      FBIT(pane->flags, kPaneDebug);
    }
    HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    if (FLAGGED(pane->flags, kPaneDebug)) {
      Indent(2);
      TITLE_WITH_TAG(pane->title, pane->tag);
      u32 hash = GetHash(title_with_tag, strlen(title_with_tag));
      snprintf(buffer, 64, "tag: %u", pane->tag);
      Text(buffer);
      snprintf(buffer, 64, "hash: %u", hash);
      Text(buffer);
      snprintf(buffer, 64, "hash_idx: %u", hash % kMaxHashPane);
      Text(buffer);
      snprintf(buffer, 64, "hidden: %i", FLAGGED(pane->flags, kPaneHidden));
      Text(buffer);
      snprintf(buffer, 64, "pane (%.2f,%.2f,%.2f,%.2f)",
               pane->rect.x, pane->rect.y, pane->rect.width,
               pane->rect.height);
      Text(buffer);
      snprintf(buffer, 64, "header (%.2f,%.2f,%.2f,%.2f)",
               pane->header_rect.x, pane->header_rect.y,
               pane->header_rect.width, pane->header_rect.height);
      Text(buffer);
      snprintf(buffer, 64, "theoretical_height (%.2f)",
               pane->theoretical_height);
      Text(buffer);
      snprintf(buffer, 64, "vscroll (%.2f)", pane->vertical_scroll);
      Text(buffer);
      imui::SameLine();
      Text("flags: "); Bitfield8(pane->flags);
      imui::NewLine();
      HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
      Indent(0);
    }
  }
  //imui::SameLine();
  //Checkbox(16.f, 16.f, &kIMUI.debug_enabled);
  //Text(" Debug Enabled");
  NewLine();
  Text("Tags");
  HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
  for (int i = 0; i < kMaxTags; ++i) {
    snprintf(buffer, 64, "Tag %i", i);
    TextOptions toptions;
    toptions.highlight_color = v4f(1.f, 0.f, 0.f, 1.f);
    if (Text(buffer, toptions).clicked) {
      kIMUI.debug_show_details[i] = !kIMUI.debug_show_details[i];
    }
    HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    if (kIMUI.debug_show_details[i]) {
      snprintf(buffer, 64, "Text Exhaustion (%u / %u)  ",
               kIMUI.text_exhaustion[i], kMaxText);
      Text(buffer);
      ProgressBar(100.f, 8.f, kIMUI.text_exhaustion[i], kMaxText,
                  v4f(1.f, 0.f, 0.f, 1.f), v4f(.3f, .3f, .3f, 1.f));
      snprintf(buffer, 64, "Button Exhaustion (%u / %u) Circle (%u / %u)",
               kIMUI.button_exhaustion[i], kMaxButton,
               kIMUI.button_circle_exhaustion[i], kMaxButtonCircle);
      Text(buffer);
      HorizontalLine(v4f(1.f, 1.f, 1.f, .2f));
    }
  }

  v2f mouse_pos = GetMousePosition();
  snprintf(buffer, 64, "Mouse Pos (%.2f,%.2f)", mouse_pos.x, mouse_pos.y);
  Text(buffer);
  v2f delta = MouseDelta();
  snprintf(buffer, 64, "Mouse Delta (%.2f,%.2f)", delta.x, delta.y);
  Text(buffer);
  snprintf(buffer, 64, "Mouse Down (%i)", imui::IsMouseDown());
  Text(buffer);
  snprintf(buffer, 64, "Mouse Wheel (%.2f)", imui::GetMouseWheel());
  Text(buffer);
  
  // This needs to run last else MouseInUI won't run correctly against this
  // panels bounds...
  SameLine();
  Text("Mouse in UI ");
  for (int i = 0; i < kMaxTags; ++i) {
    snprintf(buffer, 64, "(tag:%i, in_ui:%u) ", i, imui::MouseInUI(mouse_pos, i));
    Text(buffer);
  }
  Text(")");
  NewLine();
  End();
}

const char*
LastErrorString()
{
  unsigned err = imui_errno;
  imui_errno = 0;
  switch (err) {
    case 1:
      return ("imui text count exhausted.");
    case 2:
      return ("text provided surpasses max allowed imui character count.");
    case 3:
      return ("imui button count exhausted.");
    case 4:
      return ("imui click count exhausted.");
    case 5:
      return ("imui line count exhausted.");

  };

  return 0;
}

}  // namespace imui

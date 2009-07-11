#include "string-descriptors.h"
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[18];
} string_descriptor_1_en= {{40, 3, {'U'}}, {
'S', 'B', ' ', 'p', 's', 'e', 'u', 'd', 'o', ' ', 'e', 't', 'h', 'e', 'r', 'n', 'e', 't'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[8];
} string_descriptor_2_all= {{20, 3, {'F'}}, {
'l', 'u', 'f', 'f', 'w', 'a', 'r', 'e'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[2];
} string_descriptor_3_all= {{8, 3, {'0'}}, {
'.', '1'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[11];
} string_descriptor_4_all= {{26, 3, {'0'}}, {
'2', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1'}};
static const struct usb_st_string_descriptor * string_table_en[] =
{
  &string_descriptor_1_en.base,
  &string_descriptor_2_all.base,
  &string_descriptor_3_all.base,
  &string_descriptor_4_all.base,
};
static const struct {
  struct usb_st_language_descriptor base;
  Uint16 langs[0];
} language_descriptor =
{
  {4, 3, {0x0409}},
  {}};
static const struct {
  struct usb_st_string_languages base;
  struct  usb_st_string_language_map map[0];
} string_languages_full={{1, 4, &language_descriptor.base, 
    {{0x0409, string_table_en}}}, {
  }
};
const struct usb_st_string_languages * const string_languages = &string_languages_full.base;

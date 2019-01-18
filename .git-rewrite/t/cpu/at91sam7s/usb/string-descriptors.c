#include "string-descriptors.h"
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[15];
} string_descriptor_1_en= {{34, 3, {'S'}}, {
'e', 'r', 'i', 'a', 'l', ' ', 'i', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[8];
} string_descriptor_1_sv= {{20, 3, {'S'}}, {
'e', 'r', 'i', 'e', 'p', 'o', 'r', 't'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[8];
} string_descriptor_2_all= {{20, 3, {'F'}}, {
'l', 'u', 'f', 'f', 'w', 'a', 'r', 'e'}};
static const struct {
  struct usb_st_string_descriptor base;
  Uint16 chars[3];
} string_descriptor_3_all= {{10, 3, {'0'}}, {
'.', '0', '1'}};
static const struct usb_st_string_descriptor * string_table_en[] =
{
  &string_descriptor_1_en.base,
  &string_descriptor_2_all.base,
  &string_descriptor_3_all.base,
};
static const struct usb_st_string_descriptor * string_table_sv[] =
{
  &string_descriptor_1_sv.base,
  &string_descriptor_2_all.base,
  &string_descriptor_3_all.base,
};
static const struct {
  struct usb_st_language_descriptor base;
  Uint16 langs[1];
} language_descriptor =
{
  {6, 3, {0x0409}},
  {0x041d, }};
static const struct {
  struct usb_st_string_languages base;
  struct  usb_st_string_language_map map[1];
} string_languages_full={{2, 3, &language_descriptor.base, 
    {{0x0409, string_table_en}}}, {
    {0x041d, string_table_sv},
  }
};
const struct usb_st_string_languages * const string_languages = &string_languages_full.base;

#include "usb.h"
struct usb_st_string_language_map
{
  Uint16 lang_id;
  const struct usb_st_string_descriptor * const *descriptors;
};

struct usb_st_string_languages
{
  Uchar num_lang;
  Uchar max_index;
  const struct usb_st_language_descriptor *lang_descr;
  const struct usb_st_string_language_map map[1];
};
  
extern const struct usb_st_string_languages * const string_languages;

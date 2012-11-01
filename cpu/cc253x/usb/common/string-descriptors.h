#ifndef STRUCTGEN
#include "usb.h"
#endif
#include <stdint.h>

struct usb_st_string_language_map {
  uint16_t lang_id;
  const struct usb_st_string_descriptor *const *descriptors;
};

struct usb_st_string_languages {
  uint8_t num_lang;
  uint8_t max_index;
  const struct usb_st_language_descriptor *lang_descr;
  const struct usb_st_string_language_map map[1];
};

extern const struct usb_st_string_languages *const string_languages;

const uint8_t *usb_class_get_string_descriptor(uint16_t lang, uint8_t index);


#include "ctk-gtksim.h"

#include "ctk-gtksim-draw.h"

#define RASTER_X 8

#define FONT_BASELINE 8

struct ctk_gtksim_draw_font {
  /*  GdkFont *font;*/
  PangoFontDescription *font;
};

struct ctk_gtksim_draw_color {
  GdkGC *color;
};

struct ctk_gtksim_draw_font ctk_gtksim_draw_font_normal,
  ctk_gtksim_draw_font_bold,
  ctk_gtksim_draw_font_menu,
  ctk_gtksim_draw_font_monospace,
  ctk_gtksim_draw_font_monospace_bold;

struct ctk_gtksim_draw_color ctk_gtksim_draw_color_white,
  ctk_gtksim_draw_color_lightgray,
  ctk_gtksim_draw_color_midgray,
  ctk_gtksim_draw_color_darkgray,
  ctk_gtksim_draw_color_ddarkgray,
  ctk_gtksim_draw_color_black,
  ctk_gtksim_draw_color_blue;

/*static GdkFont *font, *ctk_gtksim_draw_font_bold, *menufont, *ctk_gtksim_draw_font_monospace;*/

static PangoLayout *layout;

/*--------------------------------------------------------------------------*/
int
ctk_gtksim_draw_string_width(struct ctk_gtksim_draw_font *font,
			     char *str, int monospace)
{
  int width;
  if(monospace) {
    return strlen(str) * RASTER_X;
  } else {
    pango_layout_set_text(layout, str, -1);
    pango_layout_set_font_description(layout, font->font);
    pango_layout_get_size(layout, &width, NULL);
    return width / PANGO_SCALE;
    /*    return gdk_string_width(font->font, str);*/
  }
}
/*--------------------------------------------------------------------------*/
void
ctk_gtksim_draw_string(struct ctk_gtksim_draw_font *font,
		       struct ctk_gtksim_draw_color *color,
		       int x, int y, char *str, 
		       int monospace)
{
  int i;
  char text[2];

  pango_layout_set_font_description(layout, font->font);
  
  if(monospace) {
    text[1] = 0;
    for(i = 0; i < strlen(str); ++i) {
      text[0] = str[i];
      pango_layout_set_text(layout, text, 1);
      
      gdk_draw_layout(ctk_gtksim_pixmap, color->color,
		      x, y, layout);
      /*    gdk_draw_string(ctk_gtksim_pixmap, font->font, color->color,
	    x, y + FONT_BASELINE, text);*/
      x += RASTER_X;
    }
  } else {
    pango_layout_set_text(layout, str, -1);
    
    gdk_draw_layout(ctk_gtksim_pixmap, color->color,
		    x, y, layout);
    /*    gdk_draw_string(ctk_gtksim_pixmap, font->font, color->color,
	  x, y + FONT_BASELINE, str);*/
  }
}
/*--------------------------------------------------------------------------*/
static GdkGC *
get_color(unsigned short r, unsigned short g, unsigned short b)
{
  GdkGCValues values;
  GdkColor color;

  color.pixel = 0;
  color.red = r;
  color.green = g;
  color.blue = b;

  if(gdk_colormap_alloc_color(gdk_colormap_get_system(),
			      &color, FALSE, TRUE)) {
  }
  
  values.foreground = color;
  
  return gdk_gc_new_with_values(ctk_gtksim_drawing_area->window,
				&values,
				GDK_GC_FOREGROUND);  
}
/*--------------------------------------------------------------------------*/
void
ctk_gtksim_draw_init(void)
{
  /*  
  ctk_gtksim_draw_font_normal.font =
    gdk_font_load("-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*");
  if(ctk_gtksim_draw_font_normal.font != NULL) {
    printf("ctk-gtksim-service: Font loaded OK\n");
  } else {
    printf("ctk-gtksim-service: Font loading failed\n");
    exit(1);
  }

  ctk_gtksim_draw_font_bold.font =
    gdk_font_load("-*-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*");
  if(ctk_gtksim_draw_font_bold.font != NULL) {
    printf("ctk-gtksim-service: Font loaded OK\n");
  } else {
    printf("ctk-gtksim-service: Font loading failed\n");
    exit(1);
  }

  ctk_gtksim_draw_font_menu.font =
    gdk_font_load("-*-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");
  if(ctk_gtksim_draw_font_menu.font != NULL) {
    printf("ctk-gtksim-service: Font loaded OK\n");
  } else {
    printf("ctk-gtksim-service: Font loading failed\n");
    exit(1);
  }

  ctk_gtksim_draw_font_monospace.font =
    gdk_font_load("-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*");
  if(ctk_gtksim_draw_font_monospace.font != NULL) {
    printf("ctk-gtksim-service: Font loaded OK\n");
  } else {
    printf("ctk-gtksim-service: Font loading failed\n");
    exit(1);
  }
  */
  ctk_gtksim_draw_color_blue.color = get_color(0, 0, 0xffff);
  ctk_gtksim_draw_color_white.color = get_color(0xffff, 0xffff, 0xffff);
  ctk_gtksim_draw_color_lightgray.color = get_color(0xefff, 0xefff, 0xefff);
  ctk_gtksim_draw_color_midgray.color = get_color(0xdfff, 0xdfff, 0xdfff);
  ctk_gtksim_draw_color_darkgray.color = get_color(0xcfff, 0xcfff, 0xcfff);
  ctk_gtksim_draw_color_ddarkgray.color = get_color(0xafff, 0xafff, 0xafff);
  ctk_gtksim_draw_color_black.color = get_color(0, 0, 0);

  layout = pango_layout_new(gtk_widget_get_pango_context(ctk_gtksim_drawing_area));

  ctk_gtksim_draw_font_normal.font =
      pango_font_description_from_string("Arial 8");
  ctk_gtksim_draw_font_bold.font =
      pango_font_description_from_string("Arial bold 8");
  ctk_gtksim_draw_font_menu.font =
      pango_font_description_from_string("Arial bold 9");
  ctk_gtksim_draw_font_monospace.font =
      pango_font_description_from_string("Monospace 8");
  ctk_gtksim_draw_font_monospace_bold.font =
      pango_font_description_from_string("Monospace bold 8");
  
  
}  
/*--------------------------------------------------------------------------*/

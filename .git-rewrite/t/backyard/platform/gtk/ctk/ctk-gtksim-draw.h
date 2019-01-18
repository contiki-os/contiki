#ifndef __CTK_GTKSIM_DRAW_H__
#define __CTK_GTKSIM_DRAW_H__

struct ctk_gtksim_draw_font;

extern struct ctk_gtksim_draw_font ctk_gtksim_draw_font_normal,
  ctk_gtksim_draw_font_bold,
  ctk_gtksim_draw_font_menu,
  ctk_gtksim_draw_font_monospace,
  ctk_gtksim_draw_font_monospace_bold;

struct ctk_gtksim_draw_color;

extern struct ctk_gtksim_draw_color ctk_gtksim_draw_color_white,
  ctk_gtksim_draw_color_lightgray,
  ctk_gtksim_draw_color_midgray,
  ctk_gtksim_draw_color_darkgray,
  ctk_gtksim_draw_color_ddarkgray,
  ctk_gtksim_draw_color_black,
  ctk_gtksim_draw_color_blue;

void ctk_gtksim_draw_init(void);

int ctk_gtksim_draw_string_width(struct ctk_gtksim_draw_font *font,
				 char *str, int monospace);

void ctk_gtksim_draw_string(struct ctk_gtksim_draw_font *font,
			    struct ctk_gtksim_draw_color *color,
			    int x, int y, char *str, 
			    int monospace);

void ctk_gtksim_draw_line(struct ctk_gtksim_draw_color *color,
			  int x1, int y1,
			  int x2, int y2);


#endif /* __CTK_GTKSIM_DRAW_H__ */

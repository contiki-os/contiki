#include "lib/list.h"

struct example_list_struct {
  struct *next;
  int number;
};

LIST(example_list);

static struct example_list_struct element1, element2;

void
example_function(void)
{
  struct example_list_struct *s;

  list_init(example_list);

  element1.number = 1;
  list_add(example_list, &element1);

  element2.number = 2;
  list_add(example_list, &element2);

  for(s = list_head(example_list);
      s != NULL;
      s = list_item_next(s)) {
    printf("List element number %d\n", s->number);
  }
}

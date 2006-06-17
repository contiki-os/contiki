#include "list.h"

struct example_list_struct {
  struct *next;
  int number;
};

LIST(example_list);

void
example_function(void)
{
  struct example_list_struct *s;
  struct example_list_struct element1, element2;

  list_init(example_list);

  list_add(example_list, &element1);
  list_add(example_list, &element2);
  
  for(s = list_head(example_list);
      s != NULL;
      s = s->next) {
    printf("List element number %d\n", s->number);
  }
  
}

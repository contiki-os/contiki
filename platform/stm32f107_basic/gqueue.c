#include <gqueue.h>

void
queue_enqueue(volatile void *q, const void *elt)
{
  volatile struct generic_queue *gq = q;

  if(gq->len == 0) {
    gq->head = gq->memory;
    gq->tail = gq->memory;
  } else {
    if(gq->tail == gq->memory + (gq->max_capacity - 1) * gq->item_size)
      /* FIXME: Should something be done about
       * queue length? Substact one, maybe?*/
      gq->tail = gq->memory;
    else
      gq->tail = (uint8_t *) gq->tail + gq->item_size;
  }

  memcpy((void *)gq->tail, elt, gq->item_size);
  gq->len++;
}

void
queue_dequeue(volatile void *q, void *elt)
{
  volatile struct generic_queue *gq = q;

  memcpy(elt, (void *)gq->head, gq->item_size);

  if(gq->head == gq->memory + (gq->max_capacity - 1) * gq->item_size)
    /* FIXME: Should something be done about
     * queue length? Substact one, maybe?*/
    gq->head = gq->memory;
  else if(gq->len > 1)
    gq->head = (uint8_t *) gq->head + gq->item_size;
  gq->len--;
}

# Cambios hecho en documentos que no son MLBOT

## tsch.c

* **add** 
  *#include "net/mac/tsch/mlbot/mlbot.h"*, en la sección de los include
* **add** 


##tsch-slot-operation.c
* **add** 
  *#include "net/mac/tsch/mlbot/mlbot.h"*, en la sección de los include
* **add**
*    #if TSCH_MLBOT_ON
        /* Aqui se acaba de poner un EB en la cola, por lo que se le informa a MLBOT */
        eb_has_send = 1;
        
        if (process_post(&mlbot_process, PROCESS_EVENT_POLL, NULL) != PROCESS_ERR_OK){
          PRINTF ("MLBOT: The event EB send could not be posted to mlbot process");
          /* !!!!! aqui se puede tomar un desición para asegurar que mlbot se entere, o no */
        }    
    #endif*

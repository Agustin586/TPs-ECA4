#ifndef INCLUDE_OFFSET_H_
#define INCLUDE_OFFSET_H_

#include "filtro.h"
#include <stdint.h>

typedef struct
{
    // Publico:
    float amp_vp;
    float offset;

    // Privado:
    filtroRC Desacople;
    filtroRC OffsetPost;
    filtroRC offsetNegt;
} offset_t;

extern void EtapaOffset_init(offset_t *offsetObj);
extern void EtapaOffset_setDesacople(offset_t *offsetObj);
extern void EtapaOffset_clearDesacople(offset_t *offsetObj);
extern void EtapaOffset_setOffsetSignal(offset_t *offsetObj);
extern void EtapaOffset_clearOffsetSignal(offset_t *offsetObj);

#endif
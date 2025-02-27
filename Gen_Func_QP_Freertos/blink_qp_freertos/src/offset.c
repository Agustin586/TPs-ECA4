#include "offset.h"

#define AMPLITUD_MAX_SALIDA 3.3
#define OFFSET_MAX 5

extern void EtapaOffset_init(offset_t *offsetObj)
{
    offsetObj->amp_vp = 0;
    offsetObj->offset = 0;

    /* Iniciamos los filtros */
    filtroRC_init(&(offsetObj->Desacople));
    filtroRC_init(&(offsetObj->OffsetPost));
    filtroRC_init(&(offsetObj->offsetNegt));

    return;
}
extern void EtapaOffset_setDesacople(offset_t *offsetObj)
{
    float val_medio = offsetObj->amp_vp * AMPLITUD_MAX_SALIDA / OFFSET_MAX;

    filtroRC_setValMedio(&(offsetObj->Desacople), val_medio);
    filtroRC_enable(&(offsetObj->Desacople));

    return;
}
extern void EtapaOffset_clearDesacople(offset_t *offsetObj)
{
    filtroRC_disable(&(offsetObj->Desacople));

    return;
}
extern void EtapaOffset_setOffsetSignal(offset_t *offsetObj)
{
    float val_medio = offsetObj->offset * AMPLITUD_MAX_SALIDA / OFFSET_MAX;

    if (offsetObj->offset > 0)
    {
        filtroRC_setValMedio(&(offsetObj->OffsetPost), val_medio);
        filtroRC_enable(&(offsetObj->OffsetPost));
        filtroRC_disable(&(offsetObj->offsetNegt));
    }
    else if (offsetObj->offset < 0)
    {
        filtroRC_setValMedio(&(offsetObj->offsetNegt), -val_medio);
        filtroRC_disable(&(offsetObj->OffsetPost));
        filtroRC_enable(&(offsetObj->offsetNegt));
    }

    return;
}
extern void EtapaOffset_clearOffsetSignal(offset_t *offsetObj)
{
    filtroRC_disable(&(offsetObj->offsetNegt));
    filtroRC_disable(&(offsetObj->OffsetPost));

    return;
}
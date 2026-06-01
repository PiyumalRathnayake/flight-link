#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

void profileChange(lv_event_t * e);
void rxChange(lv_event_t * e);
void encEvtCh(lv_event_t * e);
void sensivityevt(lv_event_t * e);
void startcal(lv_event_t * e);
void bindTXevt(lv_event_t * e);
void lowPowerEvt(lv_event_t * e);
void medPowerEvt(lv_event_t * e);
void higPowerEvt(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

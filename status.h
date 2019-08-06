#ifndef STATUS_H_
#define STATUS_H_

extern int32_t g_hunger;
extern int32_t g_happiness;
extern uint32_t g_frame_count;

extern void timer0IntHandler();
extern void timer1IntHandler();
extern void initTimer0();
extern void initTimer1();
extern void idle_animation();


#endif /* STATUS_H_ */

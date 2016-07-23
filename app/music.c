#include "scheduler.h"
#include "timer.h"

static bool is_playing = false;

#define DO_FREQ 	261
#define RE_FREQ 	293
#define MI_FREQ 	329
#define FA_FREQ 	349
#define SOL_FREQ 	392
#define LA_FREQ 	440
#define SI_FREQ 	494

#define TEMPO 		400

enum note_length {
	WHOLE_NOTE,
	HALF_QUARTER_NOTE,
	HALF_NOTE,
	QUARTER_EIGHT_NOTE,
	QUARTER_NOTE,
	EIGHT_NOTE,
	TRIPLET_NOTE,
	SIXTEENTH_NOTE,
};

#define RONDE 			WHOLE_NOTE
#define BLANCHE_NOIRE	HALF_QUARTER_NOTE
#define BLANCHE			HALF_NOTE
#define NOIRE_CROCHE 	QUARTER_EIGHT_NOTE
#define NOIRE 			QUARTER_NOTE
#define CROCHE			EIGHT_NOTE
#define TRIOLET			TRIPLET_NOTE
#define DOUBLE_CROCHE 	SIXTEENTH_NOTE

enum note {
	SILENCE,
	K4,
	DO3,
	RE3,
	MI3,
	FA3,
	SOL3,
	LA3,
	SI3,
	DO4,
	RE4,
	MI4,
	FA4,
	SOL4,
	LA4,
	SI4,
};

const int note2freq[] = {100000, 4000, 
						 261, 293, 329, 349, 392, 440, 494,/*3*/
						 523, 587, 659, 698, 784, 880, 988/*4*/};
const int note2duration[] = {4 * TEMPO, 3 * TEMPO, 2 * TEMPO, (3 * TEMPO) / 2,
						     TEMPO, TEMPO / 2, TEMPO / 3, TEMPO /4};

struct note_info {
	enum note note;
	enum note_length d;
};

#if 0
/* capucine */
const struct note_info melody[] = {{DO3, BLANCHE},
								{RE3, NOIRE}, {RE3, NOIRE},
								{SOL3, NOIRE}, {SOL3, NOIRE},
								{MI3, NOIRE}, {MI3, NOIRE},
								{DO3, BLANCHE},
								{RE3, NOIRE}, {RE3, NOIRE},
								{SOL3, NOIRE}, {SOL3, NOIRE},
								{MI3, BLANCHE},
								{DO3, BLANCHE},
								{RE3, NOIRE}, {RE3, NOIRE},
								{SOL3, NOIRE}, {SOL3, NOIRE},
								{MI3, NOIRE}, {MI3, NOIRE},
								{DO3, BLANCHE},
								{RE3, NOIRE}, {RE3, NOIRE},
								{SOL3, NOIRE}, {SOL3, NOIRE},
								{DO3, BLANCHE},
								{DO4, BLANCHE}};
#endif
#if 1
/*star wars */
const struct note_info melody[] = {
	{K4, NOIRE}, {K4, NOIRE}, {K4, NOIRE}, {K4, NOIRE},
	{FA3, TRIOLET}, {FA3, TRIOLET}, {FA3, TRIOLET},
	{SI3, BLANCHE},{FA4, NOIRE}, {MI4, TRIOLET}, {RE4, TRIOLET}, {DO4, TRIOLET},
	{SI4, BLANCHE},{FA4, NOIRE}, {MI4, TRIOLET}, {RE4, TRIOLET}, {DO4, TRIOLET},
	{SI4, BLANCHE},{FA4, NOIRE}, {MI4, TRIOLET}, {RE4, TRIOLET}, {MI4, TRIOLET},

	{DO4, BLANCHE_NOIRE}, {FA3, TRIOLET}, {FA3, TRIOLET}, {FA3, TRIOLET},
	{DO4, BLANCHE}, {SILENCE, NOIRE}, {FA3, CROCHE}, {FA3, CROCHE},
	{SOL3, NOIRE_CROCHE}, {SOL3, CROCHE}, {MI4, CROCHE}, {RE4, CROCHE}, {DO4, CROCHE}, {SI3, CROCHE},
	{SI3, CROCHE}, {DO4, DOUBLE_CROCHE}, {RE4, DOUBLE_CROCHE}, {DO4, CROCHE}, {SOL3, CROCHE}, {LA3, NOIRE}, {FA3, CROCHE}, {FA3, CROCHE},

	{SOL3, NOIRE_CROCHE}, {SOL3, CROCHE}, {MI4, CROCHE}, {RE4, CROCHE}, {DO4, CROCHE}, {SI3, CROCHE},
	{FA4, BLANCHE}, {FA3, NOIRE}, {FA3, CROCHE}, {FA3, CROCHE},
	{SOL3, NOIRE_CROCHE}, {SOL3, CROCHE}, {MI4, CROCHE}, {RE4, CROCHE}, {DO4, CROCHE}, {SI3, CROCHE},

	{SI3, CROCHE}, {DO4, DOUBLE_CROCHE}, {RE4, DOUBLE_CROCHE}, {DO4, CROCHE}, {SOL3, CROCHE}, {LA3, NOIRE}, {FA3, CROCHE}, {FA3, CROCHE},
	{SI4, CROCHE}, {LA4, DOUBLE_CROCHE}, {SOL4, CROCHE}, {FA4, DOUBLE_CROCHE}, {MI4, CROCHE}, {RE4, DOUBLE_CROCHE}, {DO4, CROCHE}, {SI3, DOUBLE_CROCHE},
	{DO4, BLANCHE_NOIRE}, {FA3, TRIOLET}, {FA3, TRIOLET}, {FA3, TRIOLET},

	{SILENCE, RONDE},
};
#endif
const int note_nb = sizeof(melody) / sizeof(melody[0]);

void play_task(int idx, int is_pause)
{
	int pause_duration = note2duration[melody[idx].d] / 8;
	int duration = note2duration[melody[idx].d] - pause_duration;

	if (is_playing) {
		if (is_pause) {
			tim14_set_freq(SILENCE);
			schedule_task(pause_duration, (task_handler) play_task, (idx + 1) % note_nb, 0, 0, 0);
		} else {
			tim14_set_freq(note2freq[melody[idx].note]);
			schedule_task(duration, (task_handler) play_task, idx, 1, 0, 0);
		}
	} else {
		tim14_disable();
	}
}
 
/* public api */
void music_start()
{
	static bool is_timer_configure_done = false;

	if (!is_playing) {
		if (!is_timer_configure_done) {
			struct timer_mode mode;

			mode.type = TIMER_FREQUENCY_GENERATOR;
#if defined(__STM32F0DISCO__) || defined(__STM32DEV__) || defined(__HCLOCK__)
			mode.u.frequency_generator.bank = GPIO_A;
			mode.u.frequency_generator.pin_nb = 4;
			mode.u.frequency_generator.alt = GPIO_AF4;
#else
#error "Unknown stm32 board"
#endif
			tim14_set_mode(&mode);
			tim14_set_freq(SILENCE);

			is_timer_configure_done = true;
		}
		tim14_enable();
		is_playing = true;
		schedule_task(1 * ms, (task_handler) play_task,0, 0, 0, 0);
	}
}

void music_stop()
{
	is_playing = false;
}

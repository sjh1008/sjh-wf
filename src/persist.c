#include <pebble.h>
#include "persist.h"

#define STATE_PERSIST_KEY 1

static AppState s_state;

// Actually write the state to persistent storage.
void prv_persist_state(void) {
	persist_write_data(STATE_PERSIST_KEY, &s_state, sizeof(s_state));
}

void persist_state(char* weath, char* loc, int32_t tmv) {
	s_state.last_display_time = tmv;
	strcpy(s_state.weath_str, weath);
	strcpy(s_state.location_str, loc);
	prv_persist_state();
}

AppState restore_state() {
	memset(&s_state, 0, sizeof(s_state));
	if (persist_exists(STATE_PERSIST_KEY)) {
		persist_read_data(STATE_PERSIST_KEY, &s_state, sizeof(s_state));
	} else {
		snprintf(s_state.weath_str, sizeof(s_state.weath_str), "%s", " ");
		//s_state.weath_str=' ';
		snprintf(s_state.location_str, sizeof(s_state.location_str), "%s", " ");
		//s_state.location_str=' ';
		s_state.last_display_time = 0;
	}
	return s_state;
}


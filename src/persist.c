#include <pebble.h>
#include "persist.h"

#define STATE_PERSIST_KEY 1

static AppState s_state;

// Actually write the state to persistent storage.
void prv_persist_state(void) {
  //persist_delete(STATE_PERSIST_KEY);
  int leng;
	leng = persist_write_data(STATE_PERSIST_KEY, &s_state, sizeof(s_state));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote %ds", (int)leng);
}

void persist_state(char* weath, char* loc, int32_t tmv) {
 //   APP_LOG(APP_LOG_LEVEL_DEBUG, "About to persist :%d:%s:%s:", (int)tmv, loc, weath);
	s_state.last_display_time = tmv;
	strncpy(s_state.weath_str, weath, sizeof(s_state.weath_str));
	strncpy(s_state.location_str, loc, sizeof(s_state.location_str));
 //   APP_LOG(APP_LOG_LEVEL_DEBUG, "About to persist :%d:%s:%s:", (int)s_state.last_display_time, s_state.weath_str, s_state.location_str);
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


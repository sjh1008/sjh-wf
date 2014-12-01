#include <pebble.h>
#include "persist.h"


#define STATE_PERSIST_KEY 1
  
typedef struct {
  struct Location* location;
  struct tm last_display_time;
} AppState;

typedef struct {
	char weath_time_buffer[10];
	char location_layer_buffer[32];
} Location;


static AppState s_state;

// Actually write the state to persistent storage.
static void prv_persist_state(void) {
  persist_write_data(STATE_PERSIST_KEY, &s_state, sizeof(s_state));
}

void persist_state(char* weath, char* loc, int32_t time) {
  prv_persist_state();
}

void restore_state() {
  memset(&s_state, 0, sizeof(s_state));
  if(persist_exists(STATE_PERSIST_KEY)) {
    persist_read_data(STATE_PERSIST_KEY, &s_state, sizeof(s_state));
  }
}

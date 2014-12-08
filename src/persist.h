#pragma once

typedef struct {
	char weath_str[10];
	char location_str[32];
	int32_t last_display_time;
} AppState;

// Record that we are currently showing stop_id
void prv_persist_state(void);

void persist_state(char* weath, char* loc, int32_t tmv);

AppState restore_state();

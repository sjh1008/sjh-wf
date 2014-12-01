#pragma once

// Record that we are currently showing stop_id
void persist_state(char* weath, char* loc, int32_t time);

// Restore the persisted UI state.
void restore_state();

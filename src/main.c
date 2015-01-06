/*
 * main.c
 *
 *  Created on: 10 Nov 2014
 *      Author: steve
 */

#include <pebble.h>
#include <persist.h>
#define KEY_TEMP 0
#define KEY_COND 1
#define KEY_LOC 2
#define KEY_LAT 3
#define KEY_LONG 4
#define KEY_TIME 5

static Window *s_main_window;
//static GFont s_time_font;
//static GFont s_weather_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static Layer *s_loc_layer;
// Store incoming information
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];
static char location_buffer[32];
static char lat_buffer[10];
static char long_buffer[10];
static char weath_time_buffer[10];
static char location_layer_buffer[64];
static int latitude;
static int longitude;
static char longhemisphere;
static char lathemisphere;
static char connected;
static int32_t tm;
static int32_t last_disp_tm;

static void callphone();

static void send_cmd(void) {
	Tuplet value = TupletInteger(1, 1);

	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);

	if (iter == NULL) {
		return;
	}

	dict_write_tuplet(iter, &value);
	dict_write_end(iter);

	app_message_outbox_send();
}

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static void set_connected() {
	connected = bluetooth_connection_service_peek() ? 'C' : 'N'; 
}

static void update_time() {
	// Get a tm structure
	time_t temp = time(NULL);
	last_disp_tm = temp;
	struct tm *tick_time = localtime(&temp);


	// Create a long-lived buffer
	static char buffer[] = "00:00";

	// Write the current hours and minutes into the buffer
	if (clock_is_24h_style() == true) {
		// Use 24 hour format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		// Use 12 hour format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}

	set_connected();
	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, buffer);
}

static void update_layer_callback(Layer *layer, GContext* ctx) {
	graphics_context_set_text_color(ctx, GColorWhite);
	GRect bounds = layer_get_frame(layer);
	graphics_draw_text(ctx, location_layer_buffer,
			fonts_get_system_font(FONT_KEY_FONT_FALLBACK),
			GRect(5, 5, bounds.size.w-10, 100), GTextOverflowModeWordWrap,
			GTextAlignmentLeft, NULL);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_time();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "In handler, %d", tick_time->tm_min);
	//if (tick_time->tm_min % 3  == 0) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "about to call app msg send");
		send_cmd();
	//}
}

static void main_window_load(Window *window) {
// Create GBitmap, then set to created BitmapLayer
	s_background_bitmap = gbitmap_create_with_resource(
			RESOURCE_ID_IMG_BACKGROUND);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window),
			bitmap_layer_get_layer(s_background_layer));

// Create time TextLayer
	s_time_layer = text_layer_create(GRect(2, 50, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text(s_time_layer, "00:00");

	// Create GFont
	//s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

	// Improve the layout to be more like a watchface
	text_layer_set_font(s_time_layer,
			fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	// Apply to TextLayer
	//text_layer_set_font(s_time_layer, s_time_font);

	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window),
			text_layer_get_layer(s_time_layer));

	// Create temperature Layer
	s_weather_layer = text_layer_create(GRect(0, 130, 144, 25));
	text_layer_set_background_color(s_weather_layer, GColorClear);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
	text_layer_set_text(s_weather_layer, " ");

	// Create location Layer
	s_loc_layer = layer_create(GRect(0, 0, 144, 50));
	layer_set_update_proc(s_loc_layer, update_layer_callback);

//text_layer_set_background_color(s_loc_layer, GColorClear);
//text_layer_set_text_color(s_loc_layer, GColorWhite);
//text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
//text_layer_set_text(s_loc_layer, " ");
	layer_add_child(window_get_root_layer(window), s_loc_layer);

	// Create second custom font, apply it and add to Window
//s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
	text_layer_set_font(s_weather_layer,
			fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
//text_layer_set_font(s_loc_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

	layer_add_child(window_get_root_layer(window),
			text_layer_get_layer(s_weather_layer));


	// Make sure the time is displayed from the start
	update_time();
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
	//text_layer_set_text(s_loc_layer, location_layer_buffer);
}

static void main_window_unload(Window *window) {
// Destroy GBitmap
	gbitmap_destroy(s_background_bitmap);
	// APP_LOG(APP_LOG_LEVEL_DEBUG, "after gbitmap destroy");
// Destroy BitmapLayer
	bitmap_layer_destroy(s_background_layer);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "after bitmap layer destroy");

// Destroy weather elements
	text_layer_destroy(s_weather_layer);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "after text layer destroy");
	//fonts_unload_custom_font(s_weather_font);
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "after weather layer destroy");
	layer_destroy(s_loc_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	// Read first item
	Tuple *t = dict_read_first(iterator);

	// For all items
	while (t != NULL) {
		// Which key was received?
		switch (t->key) {
		case KEY_TEMP:
			snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC",
					(int) t->value->int32);
			break;
		case KEY_COND:
			snprintf(conditions_buffer, sizeof(conditions_buffer), "%s",
					t->value->cstring);
			break;
		case KEY_LOC:
			snprintf(location_buffer, sizeof(location_buffer), "%s",
					t->value->cstring);
			break;
		case KEY_LAT:
			latitude = (int) t->value->uint32;
			if (latitude < 0) {
				lathemisphere = 'S';
				latitude = -latitude;
			} else {
				lathemisphere = 'N';
			}
			snprintf(lat_buffer, sizeof(lat_buffer), "%d.%02d%c",
					latitude / 1000000, (latitude / 1000) % 1000,
					lathemisphere);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Key lat %d ", latitude);
			break;
		case KEY_LONG:
			longitude = (int) t->value->uint32;
			if (longitude < 0) {
				longhemisphere = 'W';
				longitude = -longitude;
			} else {
				longhemisphere = 'E';
			}
			snprintf(long_buffer, sizeof(long_buffer), "%d.%02d%c",
					longitude / 1000000, (longitude / 1000) % 1000,
					longhemisphere);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Key long %d ", longitude);
			break;
		case KEY_TIME:
			tm = t->value->int32;
			snprintf(weath_time_buffer, sizeof(weath_time_buffer), "%d",
					(int) tm);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Key time %d ", (int)tm);
			time_t t = (time_t) tm;
			//time_t temp = time(NULL);
			struct tm *tick_time = localtime(&t);

			// Create a long-lived buffer
			//static char buffer[] = "00:00";

			// Write the current hours and minutes into the buffer
			if (clock_is_24h_style() == true) {
				//Use 24 hour format
				strftime(weath_time_buffer, sizeof("00:00"), "%H:%M",
						tick_time);
			} else {
				//Use 12 hour format
				strftime(weath_time_buffer, sizeof("00:00"), "%I:%M",
						tick_time);
			}
			break;
		default:
			// APP_LOG(APP_LOG_LEVEL_ERROR, "Key %s not recognized!", t->key);
			break;
		}

		// Look for next item
		t = dict_read_next(iterator);
	}
// Assemble full string and display
	set_connected();
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s",
			temperature_buffer, conditions_buffer);
	snprintf(location_layer_buffer, sizeof(location_layer_buffer),
			"%c, %s, %s, %s, %s", connected, location_buffer, lat_buffer, long_buffer,
			weath_time_buffer);
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
	//text_layer_set_text(s_loc_layer, location_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i - %s", reason, translate_error(reason));
}

static void outbox_failed_callback(DictionaryIterator *iterator,
		AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	APP_LOG(APP_LOG_LEVEL_DEBUG, "In failed: %i - %s", reason, translate_error(reason));
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void callphone() {
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(),
			app_message_outbox_size_maximum());
}

static void init() {
	// Create main Window element and assign to pointer
	// Check autobuild
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
				.load = main_window_load,
				.unload = main_window_unload
			});

	set_connected();

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);

	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	callphone();

}

static void deinit() {
	persist_state(weather_layer_buffer, location_layer_buffer, last_disp_tm);
	// Destroy Window
	window_destroy(s_main_window);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished destroy");
}

int main(void) {
	AppState st = restore_state();
	strncpy(location_layer_buffer,st.location_str, sizeof(location_layer_buffer));
	strncpy(weather_layer_buffer,st.weath_str, sizeof(weather_layer_buffer));
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Location '%s' weather '%s' ", location_buffer, weather_layer_buffer);
	init();
	app_event_loop();
	deinit();
}
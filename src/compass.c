/*
 * This application shows how to use the Compass API to build a simple watchface
 * that shows where magnetic north is.
 *
 * The compass background image source is:
 *    <http://opengameart.org/content/north-and-southalpha-chanel>
 *
 */

#include "pebble.h"

#define KEY_LATITUDE 0
#define KEY_LONGITUDE 1

// Group all UI elements in a global struct
static struct CompassUI {
	Window *window;
	BitmapLayer *bitmap_layer;
	GBitmap *background;
	Layer *path_layer;
	GPath *needle_north, *needle_south;
} s_ui;

// Vector paths for the compass needles
static const GPathInfo NEEDLE_NORTH_POINTS = { 3,
	(GPoint []) { { -8, 0 }, { 8, 0 }, { 0, -36 } }
};
static const GPathInfo NEEDLE_SOUTH_POINTS = { 3,
	(GPoint []) { { 8, 0 }, { 0, 36 }, { -8, 0 } }
};

// This is the function called by the Compass Service every time the compass heading changes by more than the filter (2 degrees in this example).
void compass_heading_handler(CompassHeadingData heading_data){

	// rotate needle accordingly
	gpath_rotate_to(s_ui.needle_north, heading_data.magnetic_heading);
	gpath_rotate_to(s_ui.needle_south, heading_data.magnetic_heading);

	// Modify alert layout depending on calibration state
	if(heading_data.compass_status == CompassStatusDataInvalid) {

	} else {

	}

	// trigger layer for refresh
	layer_mark_dirty(s_ui.path_layer);
}

// This is the draw callback for the path_layer function. This function will draw both compass needles.
static void path_layer_update_callback(Layer *path, GContext *ctx) {
	gpath_draw_filled(ctx, s_ui.needle_north); // north filled
	gpath_draw_outline(ctx, s_ui.needle_south); // south outlined

	GRect bounds = layer_get_frame(path); // grabbing frame of current layer
	GPoint path_center = GPoint(bounds.size.w / 2, bounds.size.h / 2); // creating center point

	graphics_fill_circle(ctx, path_center, 10); // use it to make a black, centered circle
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, path_center, 9); // then put a white circle on top
}

// Initializes the window and all the UI elements
static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);

	// Create the bitmap for the background and put it on the screen
	s_ui.bitmap_layer = bitmap_layer_create(bounds);
	s_ui.background = gbitmap_create_with_resource(RESOURCE_ID_COMPASS_BACKGROUND);
	bitmap_layer_set_bitmap(s_ui.bitmap_layer, s_ui.background);
	// Make needle background 'transparent' with GCompOpAnd
	bitmap_layer_set_compositing_mode(s_ui.bitmap_layer, GCompOpAnd);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_ui.bitmap_layer));

	// Create the layer in which we will draw the compass needles
	s_ui.path_layer = layer_create(bounds);
	//  Define the draw callback to use for this layer
	layer_set_update_proc(s_ui.path_layer, path_layer_update_callback);
	layer_add_child(window_layer, s_ui.path_layer);

	// Initialize and define the two paths used to draw the needle to north and to south
	s_ui.needle_north = gpath_create(&NEEDLE_NORTH_POINTS);
	s_ui.needle_south = gpath_create(&NEEDLE_SOUTH_POINTS);

	// Move the needles to the center of the screen.
	GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
	gpath_move_to(s_ui.needle_north, center);
	gpath_move_to(s_ui.needle_south, center);

}

// Free all memory initialized in window_load()
static void window_unload(Window *window) {
	gpath_destroy(s_ui.needle_north);
	gpath_destroy(s_ui.needle_south);
	layer_destroy(s_ui.path_layer);
	gbitmap_destroy(s_ui.background);
	bitmap_layer_destroy(s_ui.bitmap_layer);
}

// AppMessage Callbacks
static void inbox_received_callback(DictionaryIterator *iterator) {

	Tuple *t = dict_read_first(iterator);

	while(t != NULL) {
		switch(t->key) {
			case KEY_LATITUDE:
				break;
			case KEY_LONGITUDE:
				break;
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized", (int)t->key);
		}

		t = dict_read_next(iterator);
	}
}
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox Send Failed");
}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox Send Success");
}

static void init(void) {
	// initialize compass and set a filter to 2 degrees
	compass_service_set_heading_filter(2 * (TRIG_MAX_ANGLE / 360));
	compass_service_subscribe(&compass_heading_handler);

	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

	// initialize base window
	s_ui.window = window_create();
	window_set_window_handlers(s_ui.window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_set_fullscreen(s_ui.window, true);

	window_stack_push(s_ui.window, true);
}

static void deinit(void) {
	compass_service_unsubscribe();
	window_destroy(s_ui.window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

#include "pebble.h"

// DEFINES ==========
#define KEY_LATITUDE 0
#define KEY_LONGITUDE 1
#define HRS 15
#define M_PI 3.14159265358979323846
#define DR(X) ((X) * M_PI / 180.0)
#define RD(X) ((X) * 180.0 / M_PI)

// GLOBAL VAIRIABLES ==========
static int latLong[2];
static int path_angle = 0;
static bool outline_mode = false;

// STATIC STRUCTS ==========
static Window *window;
static Layer *path_layer;

static struct SundialUI {
	Window *window;
	BitmapLayer *bitmap_layer;
	GBitmap *background;
	Layer *path_layer;
} s_ui;

static GPath *tick_path;
static GPath *gnomon_path;
static GPoint ticks[HRS];
static double thla[HRS];

// PATHS ==========
static const GPathInfo GNOMON = {4,
	(GPoint []) {{-5, 0}, {5, 0}, {5,-30}, {-5,- 30}}
};

static const GPathInfo TICK = {4,
	(GPoint []) {{-8, -3}, {8, -3}, {8,3}, {-8,3}}
};

// MATH STUFF ==========

// from http://stackoverflow.com/questions/2284860/how-does-c-compute-sin-and-other-math-functions
double _pow(double a, double b) {
		double c = 1;
		int i;
		for (i=0; i<b; i++)
				c *= a;
		return c;
}
double _fact(double x) {
		double ret = 1;
		int i;
		for (i=1; i<=x; i++)
				ret *= i;
		return ret;
}
double _sin(double x) {
		double y = x;
		double s = -1;
		int i;
		for (i=3; i<=100; i+=2) {
				y+=s*(_pow(x,i)/_fact(i));
				s *= -1;
		}
		return y;
}
double _cos(double x) {
		double y = 1;
		double s = -1;
		int i;
		for (i=2; i<=100; i+=2) {
				y+=s*(_pow(x,i)/_fact(i));
				s *= -1;
		}
		return y;
}
double _tan(double x) {
		 return (_sin(x)/_cos(x));
}

// from http://www.experts-exchange.com/Other/Math_Science/Q_24377002.html
float arctan(float x){
		if( x < 0 ){ return -arctan(-x); }
		if( x > 1 ){ return M_PI/2 - arctan(1/x); }
		if( x <= 1/8.0 ){ return  0.9974133042 * x; }
		if( x <= 2/8.0 ){ return  0.004072621 + 0.964989344 * x; }
		if( x <= 3/8.0 ){ return .017899968 + 0.910336056 * x; }
		if( x <= 4/8.0 ){ return .044740546 + 0.839015512 * x; }
		if( x <= 5/8.0 ){ return .084473784 + 0.759613648 * x; }
		if( x <= 6/8.0 ){ return .134708924 + 0.679214352 * x; }
		if( x <= 7/8.0 ){ return .192103293 + 0.602631128 * x; }
		if( x <= 8/8.0 ){ return .253371504 + 0.532545304 * x; }
		else return 0;
}


// THE PROGRAM ==========

// This is the function called by the Compass Service every time the compass heading changes by more than the filter (2 degrees in this example).
void compass_heading_handler(CompassHeadingData heading_data){

	// log north angle
	APP_LOG(APP_LOG_LEVEL_ERROR, "Magnetic Heading: %d", heading_data.magnetic_heading);

	// Modify alert layout depending on calibration state
	if(heading_data.compass_status == CompassStatusDataInvalid) {

	} else {

	}

	// trigger layer for refresh
	layer_mark_dirty(s_ui.path_layer);
}


// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *me, GContext *ctx) {

	int x = layer_get_frame(me).size.w / 2;
	int y = layer_get_frame(me).size.h / 2;

	(void)me;

	// draw all the hour ticks
	for (int i = 0; i < HRS; i++) {
		graphics_draw_line(ctx, GPoint(ticks[i].x, ticks[i].y), GPoint(x, y));
		graphics_draw_line(ctx, GPoint(ticks[i].x + 1, ticks[i].y), GPoint(x, y));
		graphics_draw_line(ctx, GPoint(ticks[i].x, ticks[i].y + 1), GPoint(x, y));
		graphics_draw_line(ctx, GPoint(ticks[i].x - 1, ticks[i].y), GPoint(x, y));
		graphics_draw_line(ctx, GPoint(ticks[i].x, ticks[i].y - 1), GPoint(x, y));
	}
	// since ticks actually go to screen center, this circle covers the middle part of the screen, so we're left with only a ring of hour ticks
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_circle(ctx, GPoint(x,y), 55);

	if (outline_mode) {
		// draw outline uses the stroke color
		graphics_context_set_fill_color(ctx, GColorWhite);
		gpath_draw_filled(ctx, gnomon_path);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		gpath_draw_outline(ctx, gnomon_path);
	} else {
		// draw filled uses the fill color
		graphics_context_set_fill_color(ctx, GColorBlack);
		gpath_draw_filled(ctx, gnomon_path);
		graphics_context_set_stroke_color(ctx, GColorBlack);
		gpath_draw_outline(ctx, gnomon_path);
	}
}

// up button click
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	layer_mark_dirty(path_layer);
}

// down button click
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	layer_mark_dirty(path_layer);
}

// select button press
static void select_raw_down_handler(ClickRecognizerRef recognizer, void *context) {
	// Show the outline of the path when select is held down
	outline_mode = true;
	layer_mark_dirty(path_layer);
}

// select button release
static void select_raw_up_handler(ClickRecognizerRef recognizer, void *context) {
	// Show the path filled
	outline_mode = false;
	// Cycle to the next path
	layer_mark_dirty(path_layer);
}

// handle all the button presses
static void config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
	window_raw_click_subscribe(BUTTON_ID_SELECT, select_raw_down_handler, select_raw_up_handler, NULL);
}


// calculations for hour tick angles
// from http://rosettacode.org/wiki/Horizontal_sundial_calculations#C
static void get_tick_data(double lat, double lng, double ref) {
	double slat;
	int h;

	slat = _sin(DR(lat));
	for(h = -6; h <7; h++) {
		double hla, hra;
		hra = 15.0*h;
		hra = hra - lng + ref;
		hla = RD(arctan(slat * _tan(DR(hra))));

		//load angles from north
		thla[h+7] = hla;
	}

	thla[1] = thla[13];
	thla[13] = -thla[13];
	thla[14] = thla[13] + (thla[13] - thla[12]);
	thla[0] = -thla[14];
}

// calculations for hour tick point positions
static void fill_ticks(GPoint cen){
	 for (int i = 0; i < HRS; i++) {

		 int32_t second_angle = TRIG_MAX_ANGLE + (TRIG_MAX_ANGLE * (thla[i]/360));

		 int y = (-cos_lookup(second_angle) * (cen.x-3) / TRIG_MAX_RATIO) + cen.y;
		 int x = (sin_lookup(second_angle) * (cen.x-3) / TRIG_MAX_RATIO) + cen.x;
		 ticks[i] = GPoint(x, y);
	}
}

// AppMessage Callbacks
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

	Tuple *t = dict_read_first(iterator);

	while(t != NULL) {
		switch(t->key) {
			case KEY_LATITUDE:
				latLong[0] = (int)t->value->int32;
				break;
			case KEY_LONGITUDE:
				latLong[1] = (int)t->value->int32;
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

static void init() {

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

	s_ui.window = window_create();
	window_set_fullscreen(s_ui.window, true);
	window_stack_push(s_ui.window, true /* Animated */);

	window_set_background_color(s_ui.window, GColorWhite);

	window_set_click_config_provider(s_ui.window, config_provider);

	Layer *window_layer = window_get_root_layer(s_ui.window);
	GRect bounds = layer_get_frame(window_layer);

	GPoint center = GPoint(bounds.size.w/2, bounds.size.h/2);
	get_tick_data(40, 79, 80);
	fill_ticks(center);

	tick_path = gpath_create(&TICK);
	gnomon_path = gpath_create(&GNOMON);

	path_layer = layer_create(bounds);
	layer_set_update_proc(path_layer, path_layer_update_callback);
	layer_add_child(window_layer, path_layer);



	// Move all paths to the center of the screen
	gpath_move_to(gnomon_path, center);
}

static void deinit() {
	compass_service_unsubscribe();

	gpath_destroy(gnomon_path);
	gpath_destroy(tick_path);

	layer_destroy(path_layer);
	window_destroy(s_ui.window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}

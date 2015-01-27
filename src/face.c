#include "pebble.h"

static Window *window;

static Layer *path_layer;

// This defines graphics path information to be loaded as a path later
static const GPathInfo HOUSE_PATH_POINTS = {
  // This is the amount of points
  4,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {0, 0}, {20, 0}, {20,20}, {0,20}   
  }
}; 

static const GPathInfo GNOMON = {
  // This is the amount of points
  4,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {-5, 0}, {5, 0}, {5,-30}, {-5,- 30} 
  }
}; 

static const GPathInfo TICK = {
  // This is the amount of points
  4,
  // A path can be concave, but it should not twist on itself
  // The points should be defined in clockwise order due to the rendering
  // implementation. Counter-clockwise will work in older firmwares, but
  // it is not officially supported
  (GPoint []) {
    {-8, -3}, {8, -3}, {8,3}, {-8,3}   
  }
}; 

#define HRS 15

  
static GPath *tick_path;
static GPath *gnomon_path;
static GPoint ticks[HRS];
static double thla[HRS];
// static int thr[HRS];


static int path_angle = 0;

static bool outline_mode = false;



// This is the layer update callback which is called on render updates
static void path_layer_update_callback(Layer *me, GContext *ctx) {
  int x = layer_get_frame(me).size.w/2;
  int y = layer_get_frame(me).size.h/2;

  (void)me;
  
  // You can rotate the path before rendering

  // There are two ways you can draw a GPath: outline or filled
  // In this example, only one or the other is drawn, but you can draw
  // multiple instances of the same path filled or outline.

  
  for (int i = 0; i < HRS; i++) {
    graphics_draw_line(ctx, GPoint(ticks[i].x,ticks[i].y), GPoint(x,y));
    graphics_draw_line(ctx, GPoint(ticks[i].x+1,ticks[i].y), GPoint(x,y));
    graphics_draw_line(ctx, GPoint(ticks[i].x,ticks[i].y+1), GPoint(x,y));
    graphics_draw_line(ctx, GPoint(ticks[i].x-1,ticks[i].y), GPoint(x,y));
    graphics_draw_line(ctx, GPoint(ticks[i].x,ticks[i].y-1), GPoint(x,y));
  }
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

static int path_angle_add(int angle) {
  return path_angle = (path_angle + angle) % 360;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path counter-clockwise
  path_angle_add(-10);
  layer_mark_dirty(path_layer);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  // Rotate the path clockwise
  path_angle_add(10);
  layer_mark_dirty(path_layer);
}

static void select_raw_down_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the outline of the path when select is held down
  outline_mode = true;
  layer_mark_dirty(path_layer);
}

static void select_raw_up_handler(ClickRecognizerRef recognizer, void *context) {
  // Show the path filled
  outline_mode = false;
  // Cycle to the next path
}

static void config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_raw_click_subscribe(BUTTON_ID_SELECT, select_raw_down_handler, select_raw_up_handler, NULL);
}


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
/////////////////http://stackoverflow.com/questions/2284860/how-does-c-compute-sin-and-other-math-functions

#define M_PI 3.14159265358979323846

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
//////////http://www.experts-exchange.com/Other/Math_Science/Q_24377002.html



#define DR(X) ((X)*M_PI/180.0)
#define RD(X) ((X)*180.0/M_PI)

static void get_tick_data(double lat, double lng, double ref)
{
  double slat;
  int h;
 
  slat = _sin(DR(lat));
//  15
//    0 1  2  3  4  5  6 7 8 9 10 11 12 13 14
//     -6 -5 -4 -3 -2 -1 0 1 2  3  4  5  6
  for(h = -6; h <7; h++)
  {
    double hla, hra;
    hra = 15.0*h;
    hra = hra - lng + ref;
    hla = RD(arctan(slat * _tan(DR(hra))));

    // //load hours
    // int t = (12+h)%12;
    // if (time == 0) thr[h+7]=12;
    // else thr[h+7] = t;

    //load angles from north
    thla[h+7] = hla; 
  }
  thla[1] = thla[13];
  thla[13] = -thla[13];
  thla[14] = thla[13] + (thla[13] - thla[12]);
  thla[0] = -thla[14];
}
////////http://rosettacode.org/wiki/Horizontal_sundial_calculations#C


static void fill_ticks(GPoint cen){
   for (int i = 0; i < HRS; i++) {
//      int32_t second_angle = TRIG_MAX_ANGLE * (t.tm_sec / 60);
     int32_t second_angle = TRIG_MAX_ANGLE + (TRIG_MAX_ANGLE * (thla[i]/360));
     // int32_t second_angle = (-TRIG_MAX_ANGLE/4) + (TRIG_MAX_ANGLE/2*i/(HRS-1));
     int y = (-cos_lookup(second_angle) * (cen.x-3) / TRIG_MAX_RATIO) + cen.y;
     int x = (sin_lookup(second_angle) * (cen.x-3) / TRIG_MAX_RATIO) + cen.x;
     ticks[i] = GPoint(x, y);
  }
}


static void init() {
  
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorWhite);

  window_set_click_config_provider(window, config_provider);

  Layer *window_layer = window_get_root_layer(window);
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
  gpath_destroy(gnomon_path);
  gpath_destroy(tick_path);

  layer_destroy(path_layer);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

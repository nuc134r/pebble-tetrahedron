#define pi 3.1415926535 
#define scr_x1 0
#define scr_y1 0
#define scr_x2 144
#define scr_y2 168
#define world_x1 -72
#define world_y1 -84
#define world_x2 72
#define world_y2 84 
  
#define scale 100
  
#include <pebble.h>
#include <math.h>

Window *my_window;
TextLayer *text_layer;
float tetrahedron_coords[4][3];
GPoint screen_coords[4];
int acc_data[2];

void setInitValues() {
  tetrahedron_coords[0][0] = 0;
  tetrahedron_coords[0][1] = 0;
  tetrahedron_coords[0][2] = 0;
  
  tetrahedron_coords[1][0] = 1;
  tetrahedron_coords[1][1] = 0;
  tetrahedron_coords[1][2] = 0;
  
  tetrahedron_coords[2][0] = 0.5;
  tetrahedron_coords[2][1] = sqrt(3)/2;
  tetrahedron_coords[2][2] = 0;
  
  tetrahedron_coords[3][0] = 0.5;
  tetrahedron_coords[3][1] = sqrt(3)/6;
  tetrahedron_coords[3][2] = sqrt(6)/3;
  
  float DeltaX = 0.5;
  float DeltaY = sqrt(3)/6;
  float DeltaZ = sqrt(6)/6;
  
  for(int i = 0; i <= 3; i++)
  {
    tetrahedron_coords[i][0] -= DeltaX;
    tetrahedron_coords[i][1] -= DeltaY;
    tetrahedron_coords[i][2] -= DeltaZ;
    
    tetrahedron_coords[i][0] *= scale;
    tetrahedron_coords[i][1] *= scale;
    tetrahedron_coords[i][2] *= scale;
  }
}

static void data_handler(AccelData *data, uint32_t num_samples) {
  acc_data[1] = data->x * 90 / 1000;
  acc_data[0] = data->y * 90 / -1000;
  
  static char s_buffer[128];
  
  snprintf(s_buffer, sizeof(s_buffer), "X: %d Y: %d", acc_data[1], -acc_data[0]);
  
  text_layer_set_text(text_layer, s_buffer);
  
  layer_mark_dirty(text_layer_get_layer(text_layer));
}

static void RotateAndConvertToScreenCoords() {
  float temp[4][3];
  float tmp[4][3];
  
  for(int i = 0; i <= 3; i++)
  {
    //X rotation
    tmp[i][1] = (tetrahedron_coords[i][1] * cos(acc_data[0] * (pi / 180))) - (tetrahedron_coords[i][2] * sin(acc_data[0] * (pi / 180)));
    tmp[i][2] = (tetrahedron_coords[i][1] * sin(acc_data[0] * (pi / 180))) + (tetrahedron_coords[i][2] * cos(acc_data[0] * (pi / 180)));
    temp[i][1] = tmp[i][1];
    temp[i][2] = tmp[i][2];
    //Y rotation
    tmp[i][0] = (tetrahedron_coords[i][0] * cos(acc_data[1] * (pi / 180))) - (temp[i][2] * sin(acc_data[1] * (pi / 180)));
    tmp[i][2] = (tetrahedron_coords[i][0] * sin(acc_data[1] * (pi / 180))) + (temp[i][2] * cos(acc_data[1] * (pi / 180)));
    temp[i][0] = tmp[i][0];
    temp[i][2] = tmp[i][2];
    //Z rotation
    tmp[i][0] = (temp[i][0] * cos(0)) - (temp[i][1] * sin(0));
    tmp[i][1] = (temp[i][0] * sin(0)) + (temp[i][1] * cos(0));
    temp[i][0] = tmp[i][0];
    temp[i][1] = tmp[i][1];
  }
  
  for(int i = 0; i <= 3; i++)
  {
    screen_coords[i].x = ((temp[i][0] * ((scr_x2 - scr_x1)/(world_x2 - world_x1))) + (scr_x1 - (world_x1 * (scr_x2 - scr_x1))/(world_x2 - world_x1)));
    screen_coords[i].y = ((temp[i][1] * ((scr_y2 - scr_y1)/(world_y2 - world_y1))) + (scr_y1 - (world_y1 * (scr_y2 - scr_y1))/(world_y2 - world_y1)));
  }
}

static void layer_on_update(Layer *this_layer, GContext *ctx) {
  //clearing screen
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(this_layer), 0, GCornersAll);
  
  RotateAndConvertToScreenCoords();
  
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_line(ctx, screen_coords[0], screen_coords[1]);
  graphics_draw_line(ctx, screen_coords[1], screen_coords[2]);
  graphics_draw_line(ctx, screen_coords[2], screen_coords[0]);
  graphics_draw_line(ctx, screen_coords[3], screen_coords[0]);
  graphics_draw_line(ctx, screen_coords[3], screen_coords[1]);
  graphics_draw_line(ctx, screen_coords[3], screen_coords[2]);
}

void handle_init(void) {
  my_window = window_create();

  text_layer = text_layer_create(GRect(0, 0, 144, 60));
  Layer *layer = window_get_root_layer(my_window);
  layer_add_child(layer, text_layer_get_layer(text_layer));
  text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(text_layer, GColorClear);
  layer_set_update_proc(layer, layer_on_update);
  
  window_stack_push(my_window, true);
  
  setInitValues();
  
  accel_data_service_subscribe(1, data_handler);
  
  light_enable(true);
}

void handle_deinit(void) {
  text_layer_destroy(text_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}

/*----------------------------------------------------------------------------*/
/* PebblePointer.c                                                            */
/* (C) Robin Callender 2014                                                   */
/*----------------------------------------------------------------------------*/

#include "pebble.h"

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#define SAMPLING_RATE         ACCEL_SAMPLING_10HZ
#define SAMPLES_PER_CALLBACK  1

#define SYNC_BUFFER_SIZE      48

static Window * window;
static AppSync  sync;

static BitmapLayer * image_layer;
static GBitmap     * image;

static bool     isConnected = false;
static bool     tapSwitchState = false;
static int      syncChangeCount = 0;
static uint8_t  sync_buffer[SYNC_BUFFER_SIZE];

enum Axis_Index {
  AXIS_X = 0,
  AXIS_Y = 1,
  AXIS_Z = 2,
};

enum PebblePointer_Keys {
  PP_KEY_CMD  = 128,
  PP_KEY_X    = 1,
  PP_KEY_Y    = 2,
  PP_KEY_Z    = 3,
};

enum PebblePointer_Cmd_Values {
  PP_CMD_INVALID = 0,
  PP_CMD_VECTOR  = 1,
};

typedef struct {
  uint64_t    sync_set;
  uint64_t    sync_vib;
  uint64_t    sync_missed;
} sync_stats_t;

static sync_stats_t   syncStats = {0, 0, 0};

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static char * AppMessageResult_to_String(AppMessageResult error)
{
  switch (error) {
    case APP_MSG_OK:                          return "OK";
    case APP_MSG_SEND_TIMEOUT:                return "SEND_TIMEOUT";
    case APP_MSG_NOT_CONNECTED:               return "NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING:             return "APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS:                return "INVALID_ARGS";
    case APP_MSG_BUSY:                        return "BUSY";
    case APP_MSG_BUFFER_OVERFLOW:             return "BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED:            return "ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED:     return "CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY:               return "OUT_OF_MEMORY";
    case APP_MSG_CLOSED:                      return "CLOSED";
    case APP_MSG_INTERNAL_ERROR:              return "INTERNAL_ERROR";
    default:                                  return "unknown";
  }
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static char * DictionaryResult_to_String(DictionaryResult error)
{
  switch (error) {
    case DICT_OK:                      return "OK";
    case DICT_NOT_ENOUGH_STORAGE:      return "NOT_ENOUGH_STORAGE";
    case DICT_INVALID_ARGS:            return "INVALID_ARGS";
    case DICT_INTERNAL_INCONSISTENCY:  return "INTERNAL_INCONSISTENCY";
    case DICT_MALLOC_FAILED:           return "MALLOC_FAILED";
    default:                           return "unknown";
  }
}
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void sync_error_callback(DictionaryResult dict_error,
                                AppMessageResult error,
                                void * context)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "%s: DICT_%s, APP_MSG_%s", __FUNCTION__,
    DictionaryResult_to_String(dict_error), AppMessageResult_to_String(error));
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void sync_tuple_changed_callback(const uint32_t key,
                                        const Tuple * new_tuple,
                                        const Tuple * old_tuple,
                                        void * context)
{
  if (syncChangeCount > 0) {
    syncChangeCount--;
  }
  else {
    APP_LOG(APP_LOG_LEVEL_INFO, "unblock");
  }
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void window_load(Window * window)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "%s", __FUNCTION__);

  /* This is just a dummy structure used for initializaton only. */
  Tuplet vector_dict[] = {
    TupletInteger(PP_KEY_CMD, PP_CMD_INVALID),
    TupletInteger(PP_KEY_X, (int) 0x11111111),
    TupletInteger(PP_KEY_Y, (int) 0x22222222),
    TupletInteger(PP_KEY_Z, (int) 0x33333333),
  };

  syncChangeCount = ARRAY_LENGTH(vector_dict);

  app_sync_init( &sync,
                 sync_buffer, sizeof(sync_buffer),
                 vector_dict, ARRAY_LENGTH(vector_dict),
                 sync_tuple_changed_callback,
                 sync_error_callback,
                 NULL );

  syncStats.sync_set++;
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void window_unload(Window * window)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "%s", __FUNCTION__);

  app_sync_deinit(&sync);
}

/*----------------------------------------------------------------------------*/
/*  Handle receiving a new accelerometer set of samples. In this application  */
/*  the accelerometer is configured to indicate a sample size of one. This    */
/*  allows for a better real-time transfer of data to the remote side.        */
/*  Filter out any samples which occurred when the watch was vibrating.       */
/*                                                                            */
/*  Due to the fact that a new vector dictionary can't be send until all the  */
/*  components of the previous dictorary are process via the                  */
/*  sync_tuple_changed_callback(), it is necessary to track these componets.  */
/*  Since there are four and only four components in a dictionary, a simple   */
/*  counted-reference, syncChangeCount, is used.                              */
/*  Each time sync_tuple_changed_callback() is called, this referenc counter  */
/*  is decremented. Only when the syncChangeCount is zero will a new          */
/*  dictionary be built and sent.                                             */
/*----------------------------------------------------------------------------*/
void accel_data_callback(void * data, uint32_t num_samples)
{
  AppMessageResult result;
  AccelData * vector = (AccelData*) data;

  if (vector->did_vibrate) {
    syncStats.sync_vib++;
    return;
  }

  if (syncChangeCount > 0) {
    syncStats.sync_missed++;
    return;
  }

  /* Build the dictionary to hold this vector */
  Tuplet vector_dict[] = {
    TupletInteger(PP_KEY_CMD, PP_CMD_VECTOR),
    TupletInteger(PP_KEY_X, (int) vector->x),
    TupletInteger(PP_KEY_Y, (int) vector->y),
    TupletInteger(PP_KEY_Z, (int) vector->z),
  };

  /* Send the newly built dictionary to the remote side. */
  result = app_sync_set( &sync, vector_dict, ARRAY_LENGTH(vector_dict) );

  if (result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "app_sync_set: APP_MSG_%s",
             AppMessageResult_to_String(result));
  }
  else {
    syncChangeCount = ARRAY_LENGTH(vector_dict);
    syncStats.sync_set++;
  }
}

/*----------------------------------------------------------------------------*/
/*  This happens whenever the accelerometer service indicates new tap event.  */
/*  The nominal response for this callback depends on the virtual switch      */
/*  "tapSwitchState", which behaves as a toggle switch.                       */
/*                                                                            */
/*  When tapSwitchState is false, then the tap-tap event indicates that the   */
/*  accelerometer service should be initialized for receiving accelerometer   */
/*  data. In addition, the Bluetooth EDR sniff interval is reduced. This      */
/*  results in faster response time for sending data to the remote side,      */
/*  but also consumes more power as the watch's duty-cycle is increased.      */
/*                                                                            */
/*  When tapSwitchState is true, then the tap-tap event indicates that the    */
/*  accelerometer service should be stopped for receiving accelerometer data. */
/*  At that time any runtime statistics are dumped to the log for review.     */
/*  Finally, the Bluetooth sniff interval is set back to its normal level     */
/*  so as to conserve power when not the accelerometer-data is not active.    */
/*                                                                            */
/*  As a form of user request feedback, the vibrator is triggered, to         */
/*  indicate that the "start" or "stop" request has been recognized.          */
/*                                                                            */
/*  NOTE: Just doing a tap-tap on the face of the watch does not product a    */
/*        strong enough signal to the accelerometer to reliably generate an   */
/*        event. I have found that a rapid rotation of the wrist produces     */
/*        a more reliable result.                                             */
/*----------------------------------------------------------------------------*/
void accel_tap_callback(AccelAxisType axis, uint32_t direction)
{
  /*
   *  If currently not connected to remote side, then force switch state OFF.
   */
  if (isConnected == false) {
    tapSwitchState = false;
  }
  else {
    tapSwitchState = (tapSwitchState) ? false : true;
  }

  if (tapSwitchState == true) {
    APP_LOG(APP_LOG_LEVEL_INFO, "start sampling");
    vibes_long_pulse();
    app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
    accel_data_service_subscribe( SAMPLING_RATE,
                                  (AccelDataHandler) accel_data_callback );
  }
  else {
    app_comm_set_sniff_interval(SNIFF_INTERVAL_NORMAL);
    accel_data_service_unsubscribe();
    vibes_long_pulse();
    APP_LOG(APP_LOG_LEVEL_INFO, "stop sampling");
    APP_LOG(APP_LOG_LEVEL_INFO, "stats: sync_set:    %lu",
            (unsigned long)syncStats.sync_set);
    APP_LOG(APP_LOG_LEVEL_INFO, "stats: sync_vib:    %lu",
            (unsigned long)syncStats.sync_vib);
    APP_LOG(APP_LOG_LEVEL_INFO, "stats: sync_missed: %lu",
            (unsigned long)syncStats.sync_missed);
    syncStats.sync_set = 0;
    syncStats.sync_vib = 0;
    syncStats.sync_missed = 0;
  }
}

/*----------------------------------------------------------------------------*/
/* This is called whenever the connection state changes.                      */
/*----------------------------------------------------------------------------*/
void bluetooth_connection_callback(bool connected)
{
  isConnected = connected;

  APP_LOG(APP_LOG_LEVEL_INFO, "%sonnected", (isConnected) ? "C" : "Disc");
}

/*----------------------------------------------------------------------------*/
/*  NOTE: This app is started automatically by the remote side request for    */
/*        its activation.  You do not need start this app via the watch I/F.  */
/*----------------------------------------------------------------------------*/
int main(void)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "main: entry:  %s %s", __TIME__, __DATE__);

  /*
   *   Commission App
   */
  window = window_create();

  WindowHandlers handlers = {.load = window_load, .unload = window_unload };
  window_set_window_handlers(window, handlers);

  const bool animated = true;
  window_stack_push(window, animated);

  Layer * window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  /* Display the simple splash screen to indicate PebblePointer is running. */
  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PEBBLEPOINTER);
  image_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

  /* Basic accelerometer initialization.  Enable Tap-Tap functionality. */
  accel_service_set_sampling_rate( SAMPLING_RATE );
  accel_tap_service_subscribe( (AccelTapHandler) accel_tap_callback );
  app_message_open(SYNC_BUFFER_SIZE, SYNC_BUFFER_SIZE);

  /* Request notfication on Bluetooth connectivity changes.  */
  bluetooth_connection_service_subscribe( bluetooth_connection_callback );
  isConnected = bluetooth_connection_service_peek();
  APP_LOG(APP_LOG_LEVEL_INFO, "initially %sonnected", (isConnected) ? "c" : "disc");

  /*
   *   Event Processing
   */
  app_event_loop();

  /*
   *   Decommission App
   */
  if (tapSwitchState == true) {
    accel_data_service_unsubscribe();
  }

  /* Remove the Tap-Tap callback */
  accel_tap_service_unsubscribe();

  /* Release splash-screen resources */
  gbitmap_destroy(image);
  bitmap_layer_destroy(image_layer);
  window_destroy(window);

  APP_LOG(APP_LOG_LEVEL_INFO, "main: exit");
}


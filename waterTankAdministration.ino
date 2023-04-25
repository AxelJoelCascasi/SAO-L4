//Includes

#include <Servo.h>


// Habilitacion de debug para la impresion por el puerto serial ...
//----------------------------------------------
#define SERIAL_DEBUG_ENABLED 1

#if SERIAL_DEBUG_ENABLED
  #define DebugPrint(str)\
      {\
        Serial.println(str);\
      }
#else
  #define DebugPrint(str)
#endif

#define DebugPrintEstado(tipo, estado, evento)\
      {\
        String est = estado;\
        String evt = evento;\
        String type = tipo;\
        String str;\
        str = "-----------------------------------------------------";\
        DebugPrint(str);\
        DebugPrint(type);\
        str = "EST-> [" + est + "]: " + "EVT-> [" + evt + "].";\
        DebugPrint(str);\
        str = "-----------------------------------------------------";\
        DebugPrint(str);\
      }
//----------------------------------------------

#define SERIAL_BAUDS 9600
#define MAX_STATES                    4
#define MAX_EVENTS                    6

//---------------------------------------------------------

//constantes

//RGB
const int LED_RED_PIN      = 5;
const int LED_BLUE_PIN     = 6;
const int LED_GREEN_PIN    = 7;

//HCSR04
const int DIST_SENSOR_TRIG = 2;
const int DIST_SENSOR_ECHO = 3;


//POTENTIOMETER
const int POTENTIOMETER    = 1;

// Rango mapeo de potenciometro simulando el caudalimetro
const int MIN_ANALOG_VALUE = 0;
const int MAX_ANALOG_VALUE = 1023;
const int MAX_FLOW_VALUE   = 100;
const int MIN_FLOW_VALUE   = 0;

//Sensor de ultrasonido
//velocidad del sonido, en microsegundos por centímetros
const int SOUND_SPEED = 58.2;
//water tank levels in cm
const int EMPTY_TANK = 0;
const int LOW_WATER_LEVEL    = 50 ;
const int MEDIUM_WATER_LEVEL = 100;
const int HIGH_WATER_LEVEL   = MEDIUM_WATER_LEVEL + 1 ;

const int SERVO_1_PIN              = 8 ;
const int WATER_GATE_OPENING_ANGLE = 90;
const int WATER_GATE_CLOSE_ANGLE = 0;


const int RELAY_PIN = 12;
//-----------

Servo servo_1;
//------------------------------------------------------------


 typedef void (*transition)();
  
  //Actions
  void reset      ();
  void error      ();
  void none       ();
  void raining    ();
  void liquid     ();
  void noliquid   ();

  void alert_error          (); //Notifica que no hay agua en la fuente
  void finish_loading       (); //Finaliza la carga, apaga la bomba y cierra la compuerta
  void open_water_gate      (); //Abre la compuerta
  void close_water_gate     (); //Cierra la compuerta
  void turn_on_water_pump   (); //Prende la bomba
  void turn_off_water_pump  (); //Apaga la bomba



//States
enum states          {  ST_GATE_CLOSED  ,  ST_GATE_OPENED ,  ST_PRESSURIZED_LOADED ,  ST_ERROR } current_state;
String states_s [] = { "ST_GATE_CLOSED", "ST_GATE_OPENED", "ST_PRESSURIZED_LOADED", "ST_ERROR"};
enum states last_state;

//Events
enum events            {   EV_CONT ,  EV_LOW_WATER_TANK , EV_NO_PRESSURE ,  EV_HAVE_SOURCE ,   EV_HAVENT_SOURCE ,  EV_UNK      } new_event;
  String events_s [] = {  "EV_CONT", "EV_LOW_WATER_TANK","EV_NO_PRESSURE", "EV_HAVE_SOURCE", "EV_HAVENT_SOURCE", "EV_UNKNOW"  };
enum events last_event;

/*
  transition state_table_actions[MAX_STATES][MAX_EVENTS] =
  {
    {none           , none              , none             , none            , none              , none           } , // state ST_GATE_CLOSED
    {none           , none              , none             , start_pump      , stop_pump         , liquid         } , // state ST_GATE_OPENED
    {none           , none              , none             , start_pump      , stop_pump         , none           } , // state ST_PRESSURIZED_LOADED
    {start_pump     , raining           , none             , none            , stop_pump         , none           } , // state ST_ERROR
  
    //EV_CONT       , EV_LOW_WATER_TANK , EV_NO_PRESSURE   , EV_HAVE_SOURCE  , EV_HAVENT_SOURCE  , EV_UNKNOW 
  };
  */
/*  
  states state_table_next_state[MAX_STATES][MAX_EVENTS] =
  {
    {ST_IDLE          , ST_INIT       , ST_INIT          , ST_INIT          , ST_INIT       , ST_INIT          , ST_INIT       , ST_INIT       , ST_INIT          , ST_ERROR } , // state ST_INIT
    {ST_IDLE          , ST_RAINING    , ST_IDLE          , ST_DRAWING_WATER , ST_IDLE       , ST_DRAWING_WATER , ST_IDLE       , ST_IDLE       , ST_IDLE          , ST_ERROR } , // state ST_IDLE
    {ST_RAINING       , ST_RAINING    , ST_IDLE          , ST_RAINING       , ST_RAINING    , ST_RAINING       , ST_RAINING    , ST_RAINING    , ST_RAINING       , ST_ERROR } , // state ST_RAINING
    {ST_DRAWING_WATER , ST_RAINING    , ST_DRAWING_WATER , ST_DRAWING_WATER , ST_IDLE       , ST_DRAWING_WATER , ST_IDLE       , ST_PUMP_BREAK , ST_DRAWING_WATER , ST_ERROR } , // state ST_DRAWING_WATER
    {ST_PUMP_BREAK    , ST_PUMP_BREAK , ST_PUMP_BREAK    , ST_PUMP_BREAK    , ST_PUMP_BREAK , ST_PUMP_BREAK    , ST_IDLE       , ST_PUMP_BREAK , ST_IDLE          , ST_ERROR } , // state ST_PUMP_BREAK
    {ST_INIT          , ST_INIT       , ST_INIT          , ST_INIT          , ST_INIT       , ST_INIT          , ST_INIT       , ST_INIT       , ST_INIT          , ST_ERROR }   // state ST_ERROR

    //EV_CONT         , EV_RNG        , EV_RNNG          , EV_CSP           , EV_CSTP       , EV_LWD           , EV_NLD        , EV_PRTHE      , EV_PRETHE        , EV_UNK
  };

*/
//-----------------------------------------------------

/*
*Inicializamos los pins
*/
void initialize_sistem()
{
  pinMode(LED_RED_PIN     , OUTPUT);
  pinMode(LED_GREEN_PIN   , OUTPUT);
  pinMode(LED_BLUE_PIN    , OUTPUT);
 
  pinMode(DIST_SENSOR_TRIG, OUTPUT);
  pinMode(DIST_SENSOR_ECHO, INPUT);
  
  pinMode(RELAY_PIN       , OUTPUT); 
  
  servo_1.attach(SERVO_1_PIN);

}


int get_water_flow()
{
  int waterFlowValue = analogRead(POTENTIOMETER);
  return map(waterFlowValue, MIN_ANALOG_VALUE, MAX_ANALOG_VALUE, MIN_FLOW_VALUE, MAX_FLOW_VALUE);
}

//Esta bien definir las variables adentro? sacar afuera
double get_distance_to_the_water()
{
  digitalWrite(DIST_SENSOR_TRIG,HIGH);
  
  digitalWrite(DIST_SENSOR_TRIG,LOW);
  double duracion  = pulseIn(DIST_SENSOR_ECHO,HIGH);
  double distancia = duracion/SOUND_SPEED;
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println("cm ");
 
  
  return distancia;
  
}


bool is_low_level(double result, int min) 
{
	return result > EMPTY_TANK && result <= min;
}

bool is_mid_level(double result, int value1 ,int value2 ) 
{
	return result > value1 && result <= value2;
}
bool is_high_level(double result, int max) 
{
	return result > max;
}

void show_water_tank_level()
{
   int result = get_distance_to_the_water();
   if (is_low_level(result,LOW_WATER_LEVEL))
   {
   	//Prendo led rojo
     turn_on_red_led();
   }
  if (is_mid_level(result,LOW_WATER_LEVEL,MEDIUM_WATER_LEVEL))
   {
   	//Prendo led amarillo
    turn_on_yellow_led();
   }
  if (is_high_level(result,HIGH_WATER_LEVEL))
   {
   	//Prendo led verde
    turn_on_green_led();
   }
}

//RGB functions
void Color(int R, int G, int B)
{     
     analogWrite(LED_RED_PIN , R) ;    // Red    - Rojo
     analogWrite(LED_GREEN_PIN, G) ;   // Green  - Verde
     analogWrite(LED_BLUE_PIN, B) ;    // Blue   - Azul
}

//------------Actions implementation------------
//SERVO functions
void open_water_gate()
{
  servo_1.write(WATER_GATE_OPENING_ANGLE);
}
void close_water_gate()
{
  servo_1.write(WATER_GATE_CLOSE_ANGLE);
}

//WATER PUMP functions
void turn_on_water_pump()
{
  digitalWrite(RELAY_PIN, HIGH);
}
void turn_off_water_pump()
{
  digitalWrite(RELAY_PIN, LOW);
}


void turn_on_green_led()
{
  Color ( 0, 255, 0);
}
void turn_on_yellow_led()
{
  Color ( 255, 255, 0);
}
void turn_on_red_led()
{
  Color ( 255, 0, 0);
}

//---------------------------
void setup()
{
  initialize_sistem();
  Serial.begin(SERIAL_BAUDS);
}

void loop()
{
  Serial.println(get_water_flow());
  show_water_tank_level();
  open_water_gate();
  turn_on_water_pump();

}

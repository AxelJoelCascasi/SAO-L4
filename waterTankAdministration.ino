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


//water tank levels in cm

const int LOW_WATER_LEVEL    = 50 ;
const int MEDIUM_WATER_LEVEL = 100;
const int HIGH_WATER_LEVEL   = MEDIUM_WATER_LEVEL + 1 ;




//------------------------------------------------------------

/*
*Inicializamos los pins
*/
void initialize_pins()
{
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
 
  pinMode(DIST_SENSOR_TRIG, OUTPUT);
  pinMode(DIST_SENSOR_ECHO, INPUT);

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
  double distancia = duracion/58.2;
  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println("cm ");
 
  
  return distancia;
  
}

bool is_low_level(double result, int min) 
{
	return result > 0 && result <= min;
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




void Color(int R, int G, int B)
 {     
     analogWrite(LED_RED_PIN , R) ;   // Red    - Rojo
     analogWrite(LED_GREEN_PIN, G) ;   // Green - Verde
     analogWrite(LED_BLUE_PIN, B) ;   // Blue - Azul
 }


void turn_on_green_led()
{
  Color ( 0, 255, 0);
}

/*
enciende el led en color amarillo
*/
void turn_on_yellow_led()
{
  Color ( 255, 255, 0);
}

/*
enciende el led en color rojo
*/
void turn_on_red_led()
{
  Color ( 255, 0, 0);
}

void setup()
{
  initialize_pins();
  Serial.begin(SERIAL_BAUDS);
}

void loop()
{
  Serial.println(get_water_flow());
  show_water_tank_level();
}

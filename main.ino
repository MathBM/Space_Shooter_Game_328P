#include <LiquidCrystal.h>

#define tst_bit(Y,bit_x)  (Y&(1<<bit_x)) 
#define set_bit(Y, bit_x) (Y|=(1<<bit_x))
#define clr_bit(Y, bit_x) (Y&=~(1<<bit_x))
#define cpl_bit(Y, bit_x) (Y^=(1<<bit_x))

#define UpBT 5
#define DownBT 4
#define LtBT 7
#define RtBT 6
#define ShootBT 3
#define PauseBT 2


LiquidCrystal lcd (13, 12, 11, 10, 9, 8);

int points = 0;
float energy_points = 0;
int velocity = 0;
int ship_position[2], asteroid_position[2], energy_position[2], shoot_position[2] = {0, 0};

byte space_ship[8] ={B11000, B01100, B01110, B01111, B01111, B01110, B01100, B11000};
byte asteroid[8]= {B00000, B00100, B01110, B10111, B11101, B01110, B00100, B00000};
byte explosion[8] = {B10001, B10101, B01010, B10100, B00101, B01010, B10101, B10001};
byte energy[8] = {B01110,B11011,B10001,B10101,B10101,B10101,B10001,B11111};
byte shoot[8] = {B00000,B00000,B00000,B00111,B00111,B00000,B00000,B00000};

// Auxilary Controle variables.
bool game = false;
bool flag_first = true;
bool vshoot = false;

void setup()
{
    DDRD = 0b00000011; // REGS IN --> 0 Out --> 1)
	PORTD = 0b11111100; // pull Down

    asteroid_position[0] = 12;
    shoot_position[0] = -1;
    energy_points = 100;
    velocity = 100;

    lcd.createChar(1, space_ship);
    lcd.createChar(2, asteroid);
    lcd.createChar(3, explosion);
    lcd.createChar(4, energy);
    lcd.createChar(5, shoot);

    lcd.begin(16,2);
    lcd.clear();

    Serial.begin(9600);

}

void loop()
{
    if(game)
    {
        energy_points -= 0.25;
        lcd.clear();
        painel(13);
      
        if(!(tst_bit(PIND, UpBT)))
        {
            ship_position[1] = 0;
        }
        if(!(tst_bit(PIND, DownBT)))
        {
            ship_position[1] = 1;
        }
      	if(!(tst_bit(PIND, LtBT)))
        {
            ship_position[0] -= 1;
        }
        if(!(tst_bit(PIND, RtBT)))
        {
            ship_position[0] += 1;
        }
        if(!(tst_bit(PIND, ShootBT)))
        {
            shoot_position[0] = (ship_position[0]+1);
            shoot_position[1] = ship_position[1];
            vshoot = true;
        }
        
        asteroid_position[0] -= 1;

        Draw(ship_position, 1);
        Draw(asteroid_position, 2);

        if (vshoot)
        {
          Draw(shoot_position, 5);
          shoot_position[0] += 1;	
        }

        if(asteroid_position[0] == 0)
        {
           asteroid_position[0] = 12;
           asteroid_position[1] = random(0,2);
        }
        if (shoot_position[0] == 12)
        {
            vshoot = false;
            shoot_position[0] = -1;
        }

        // DEBUG
        Serial.print("Ship Position x: ");
        Serial.print(ship_position[0], DEC);
        Serial.println("");
        Serial.print("Ship Position y: ");
        Serial.print(ship_position[1], DEC);
        Serial.println("");
        Serial.print("Asteroid Position x: ");
        Serial.print(asteroid_position[0], DEC);
        Serial.println("");
        Serial.print("Asteroid Position y: ");
        Serial.print(asteroid_position[1], DEC);
      	Serial.println("");
        
        // DEBUG

        // Colisions

        if((colisionShootAsteroid(shoot_position, asteroid_position) == 1) && (vshoot == true))
        {
            vshoot = false;
            shoot_position[0] = -1;
            animationExplosion(asteroid_position, 3);
            asteroid_position[0] = 12;
            asteroid_position[1] = random(0,2);
            points += 1;
        }
        
        if (colisionShipAsteroid(ship_position, asteroid_position) == 1)
         {
            animationExplosion(ship_position, 3);
            lcd.clear();
            game = false;
         }
        delay(100); // velocity
    }
    else
    {
        if (flag_first == 1)
        {
             while(tst_bit(PIND, ShootBT))
            {
                initialScreen();
            }
            flag_first = false;
          	lcd.clear();
            game_reset();
        }
        else
        {
            finalScreen("You Lse");
        }

        if(!(tst_bit(PIND, ShootBT)))
        {
            game_reset();
        }
    }

}

void Draw(int position[2], int numb_char)
{
    lcd.setCursor(position[0], position[1]);
    lcd.write(numb_char);
}

void animationExplosion(int position[2], int numb_char)
{
    lcd.setCursor(position[0], position[1]);
    lcd.write(3);
    delay(1000);
    lcd.clear();
}

void game_reset() 
{
    ship_position[0], ship_position[1] = 0;
    asteroid_position[0] = 12;
    asteroid_position[1] = random(0, 2);
    points = 0;
    energy_points = 100;
    game =true;
}

void painel (int positionx)
{
    lcd.setCursor(positionx, 0);
    lcd.print(points);
    lcd.setCursor(positionx, 1);
    lcd.print(energy_points);
}

void finalScreen(char text[10]) // You Lse or You Win
{
    lcd.setCursor(9, 0);
    lcd.print("Pts:");
    lcd.setCursor(13, 0);
    lcd.print(points);
    lcd.setCursor(1, 0);
    lcd.print(text);
    lcd.setCursor(0, 1);
    lcd.print("Press BT Shoot.");
}

void initialScreen()
{
    lcd.setCursor(0, 0);
    lcd.print("Press BT Shoot"); 
    lcd.setCursor(0, 1);
    lcd.print("to Start Game.");
}

int colisionShootAsteroid (int position_ob1[2], int position_ob2[2])
{
    if (((position_ob1[0] == position_ob2[0]+1) && (position_ob1[1] == position_ob2[1])) || ((position_ob1[0] == position_ob2[0]) && (position_ob1[1] == position_ob2[1])))

    {
        return 1;
    }
   
}
int colisionShipAsteroid(int position_ob1[2], int position_ob2[2])
{
    if ((position_ob1[0] == position_ob2[0]) && (position_ob1[1] == position_ob2[1]))
    {
        return 1;
    }
}   
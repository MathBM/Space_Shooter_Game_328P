#include <LiquidCrystal.h>

#define tst_bit(Y,bit_x)  (Y&(1<<bit_x)) 
#define set_bit(Y, bit_x) (Y|=(1<<bit_x))
#define clr_bit(Y, bit_x) (Y&=~(1<<bit_x))
#define cpl_bit(Y, bit_x) (Y^=(1<<bit_x))

#define UpBT 7
#define DownBT 6
#define ShootBT 5
#define PauseBT 4

LiquidCrystal lcd (13, 12, 11, 10, 9, 8);

int energy_points, points = 0;
int velocity = 100;
int ship_position[2], asteroid_position[2], energy_position[2], shoot_position[2] = {0, 0};

byte space_ship[8] ={B11000, B01100, B01110, B01111, B01111, B01110, B01100, B11000};
byte asteroid[8]= {B00000, B00100, B01110, B10111, B11101, B01110, B00100, B00000};
byte explosion[8] = {B10001, B10101, B01010, B10100, B00101, B01010, B10101, B10001};
byte energy[8]= {B01110,B11011,B10001,B10101,B10101,B10101,B10001,B11111};
byte shoot[8] = {B00000,B00000,B00000,B00111,B00111,B00000,B00000,B00000};

bool game = false;
bool flag = true;

void setup()
{
    DDRD = 0x00000000; // REGS IN (0 In 1 Out)
    PORTD = 0x00000000; // No pull Down

    asteroid_position[0] = 12;
    shoot_position[0] = -1;
    energy_points = 100;

    lcd.createChar(1, space_ship);
    lcd.createChar(2, asteroid);
    lcd.createChar(3, explosion);
    lcd.createChar(4, energy);
    lcd.createChar(5, shoot);

    lcd.begin(16,2);
    lcd.clear();
    game = false;

}

void loop()
{
    if(game)
    {


    }
    else
    {
        if (flag == 1)
        {
            do
            {
                initialScreen();
            }while(!(tst_bit(PIND, ShootBT)));
            flag = 0;
        }
        else
        {
            finalScreen("You Win");
        }
        
        if(tst_bit(PIND, ShootBT))
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

void shipExplosion(int position[2], int numb_char)
{
    lcd.clear();
    lcd.setCursor(position[0], position[1]);
    lcd.write(numb_char);
    delay(1000);
    // lcd.clear();
}

void asteriodExplosion(int position[2], int numb_char)
{
    lcd.setCursor(position[0], position[1]);
    lcd.write(numb_char);
    delay(1000);
    // lcd.clear();
}

void game_reset() 
{
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

void finalScreen(char text[10]) // You Loose or You Win
{
    lcd.setCursor(9, 0);
    lcd.print("Points:");
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
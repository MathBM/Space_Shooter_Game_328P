//! Archive Description.
/*!
* \author Matheus Barbi. M.
* \since 26/02/2022
* \version 1.0
*/

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

// Auxilary Controle variables.
#define gamestart 0
#define flag_first 1
#define vshoot 2
#define vbattery 3
#define win 4
#define pause 5

char flag = 0x00000010;

LiquidCrystal lcd (13, 12, 11, 10, 9, 8);

int points = 0;
float energy_points = 0;
int ship_position[2], asteroid_position[2], energy_position[2], shoot_position[2] = {0, 0};

// Char of objects
byte space_ship[8] ={B11000, B01100, B01110, B01111, B01111, B01110, B01100, B11000};
byte asteroid[8]= {B00000, B00100, B01110, B10111, B11101, B01110, B00100, B00000};
byte explosion[8] = {B10001, B10101, B01010, B10100, B00101, B01010, B10101, B10001};
byte energy[8] = {B01110,B11011,B10001,B10101,B10101,B10101,B10001,B11111};
byte shoot[8] = {B00000,B00000,B00000,B00111,B00111,B00000,B00000,B00000};

// Difficult variables
int velocity = 0;
int recpoints = 0;
int dropchance = 0;
int temp_aux = 0;
bool disp_up = 0;

ISR(INT0_vect)
{
    cpl_bit(flag, pause);
}

ISR(TIMER1_COMPA_vect)
{
    temp_aux += 1;

    if (temp_aux == 4)
    {
        disp_up = 1;
    }
    
}


void setup()
{
    //cli();

    DDRD = 0b00000111; // REGS IN --> 0 Out --> 1)
	PORTD = 0b11111000; // pull Down

    EICRA |=  (1<< ISC01) | (1<< ISC00);
    EIMSK |= (1<<INT0);

    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12)|(1<<CS10)|(1 << CS12);
    OCR1A = 35400;

    //sei();

    asteroid_position[0] = 12;
    asteroid_position[1] = random(0, 2);

    shoot_position[0] = -1;

    energy_points = 100;
    velocity = 300;
    recpoints = 40;
    dropchance = 50;

    lcd.createChar(1, space_ship);
    lcd.createChar(2, asteroid);
    lcd.createChar(3, explosion);
    lcd.createChar(4, energy);
    lcd.createChar(5, shoot);

    lcd.begin(16,2);
    lcd.clear();

}

void loop()
{
  while(tst_bit(flag, pause))
  {
   	lcd.setCursor(0, 0);
    lcd.print("GAME PAUSED");
    lcd.setCursor(0, 1);
    lcd.print("PRESS PAUSE BT");
    delay(500);
    lcd.clear();
    
  }
  if(tst_bit(flag, gamestart))
    {
        energy_points -= 0.20;
        if (energy_points <= 0)
            {
                animationExplosion(ship_position);
                clr_bit(flag, gamestart);
            }

        lcd.clear();
        painel(12);

        // Movement

        if(!(tst_bit(PIND, UpBT)))
        {
            ship_position[1] = 0;
        }
        if(!(tst_bit(PIND, DownBT)))
        {
            ship_position[1] = 1;
        }
      	if((!(tst_bit(PIND, LtBT))) && (ship_position[0] != 0))
        {
            ship_position[0] -= 1;
        }
        if((!(tst_bit(PIND, RtBT))) && (ship_position[0] != 12))
        {
            ship_position[0] += 1;
        }
        if(!(tst_bit(PIND, ShootBT)))
        {
            shoot_position[0] = (ship_position[0]+1);
            shoot_position[1] = ship_position[1];
            set_bit(flag, vshoot);
        }
        
        asteroid_position[0] -= 1;
        // Movement //

        if(disp_up)
        {
            Draw(asteroid_position, 2);
            Draw(ship_position, 1);
        }

        if (tst_bit(flag, vshoot))
        {
          Draw(shoot_position, 5);
          shoot_position[0] += 1;	
        }

        if(asteroid_position[0] == -1)
        {
           asteroid_position[0] = 12;
           asteroid_position[1] = random(0,2);
        }
        if (shoot_position[0] == 12)
        {
            clr_bit(flag, vshoot);
            shoot_position[0] = -1;
        }

        /// Colisions
        if (colisionShipAsteroid(ship_position, asteroid) == 1)
         {
            //delay(100);
            animationExplosion(ship_position);
            lcd.clear();
            clr_bit(flag, gamestart);
         }

        if((colisionShootAsteroid(shoot_position, asteroid_position) == 1) && (tst_bit(flag, vshoot)))
        {
            clr_bit(flag, vshoot);
            shoot_position[0] = -1;
            animationExplosion(asteroid_position);
            asteroid_position[0] = 12;
            asteroid_position[1] = random(0,2);
            points += 1;
            if (points >= 2)
            {
                set_bit(flag, win);
                clr_bit(flag, gamestart);
            }
        }
        /// Colisions

        if (!(tst_bit(flag, vbattery)))
        {
            if(random(0, 101) > dropchance)
            {
                set_bit(flag, vbattery);
                energy_position[0] = 12;
                energy_position[1] = random(0, 2);

            }
        }
        else
        {
            energy_position[0] -= 1;
            Draw(energy_position, 4);
            if (colisionShootAsteroid(ship_position, energy_position) == 1)
            {
                clr_bit(flag, vbattery);
                energy_position[0] = -2;
                energy_points += 40;
            }
        }
        //delay(velocity); // Game Velocity
    }
    else
    {
        if (!tst_bit(flag, flag_first))
        {
             while(tst_bit(PIND, ShootBT))
            {
                initialScreen();
            }
            cpl_bit(flag, flag_first);
          	lcd.clear();
            game_reset();
            TIMSK1 |= (1 << OCIE1A);
        }
        else
        {
            if (tst_bit(flag, win))
            {
                velocity -= 50;
                recpoints -= 10;
                dropchance += 10; 
                energy_points = 70;
               	finalScreen("You win");
            }
            else
            {
              	velocity = 300;
                recpoints = 40 ;
                dropchance = 50; 
                energy_points = 100;
                finalScreen("You Lse");
            }
        }

        if(!(tst_bit(PIND, ShootBT)))
        {
            game_reset();
        }
    }

}

///! Draw Objects of Game in Screen.
/*!
    \param None.
    \return None.
*/
void Draw(int position[2], int numb_char)
{
    lcd.setCursor(position[0], position[1]);
    lcd.write(numb_char);
}

///! Draw Explosion about Colisions of game.
/*!
    \param position array, create explosion in this position.
    \return None.
*/
void animationExplosion(int position[2])
{
    lcd.setCursor(position[0], position[1]);
    lcd.write(3);
    delay(1000);
    lcd.clear();
}

///! Change Game States for restart.
/*!
    \param None.
    \param None.
    \return None.
*/
void game_reset() 
{
    ship_position[0] = 0; 
    ship_position[1] = 0;
    asteroid_position[0] = 12;
    asteroid_position[1] = random(0, 2);
    points = 0;
  	set_bit(flag, gamestart);
  	clr_bit(flag, win);
    
}

///! Creates Painel Of Game.
/*!
    \param positionx int, location x of painel.
    \param None.
    \return None.
*/
void painel (int positionx)
{
    lcd.setCursor(positionx, 0);
    lcd.print(points);
    lcd.setCursor(positionx, 1);
    lcd.print(energy_points);
}

///! Creates End Screen of Game.
/*!
    \param text char, can be You Lse or You Win.
    \param None.
    \return None.
*/
void finalScreen(char text[10]) 
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

///! Creates Initial Screen of Game.
/*!
    \param None.
    \param None.
    \return None.
*/
void initialScreen()
{
    lcd.setCursor(0, 0);
    lcd.print("Press BT Shoot"); 
    lcd.setCursor(0, 1);
    lcd.print("to Start Game.");
}

///! Crate Colision Shoot With Asteroid and Ship with Battery.
/*!
    \param position_ob1 array of position with 2 values.
    \param position_ob2 array of position with 2 values.
    \return one interger with value 1.
*/
int colisionShootAsteroid (int position_ob1[2], int position_ob2[2])
{
    if (((position_ob1[0] == position_ob2[0]+1) && (position_ob1[1] == position_ob2[1])) || ((position_ob1[0] == position_ob2[0]) && (position_ob1[1] == position_ob2[1])))
    {
        return 1;
    }
   
}

///! Create Colision Ship With Asteroid.
/*!
    \param position_ob1 array of position with 2 values.
    \param position_ob2 array of position with 2 values.
    \return one interger with value 1.
*/
int colisionShipAsteroid(int position_ob1[2], int position_ob2[2])
{
    if ((position_ob1[1] == position_ob2[1]) && (position_ob1[0] == position_ob2[0]))
    {
        return 1;
    }
}   
//! Archive Description.
/*!
* \author Matheus Barbi. M.
  \author Rodri Jost. 
* \since 26/02/2022
* \version 2.0
*/

#include <LiquidCrystal.h>

#define tst_bit(Y,bit_x)  (Y&(1<<bit_x)) 
#define set_bit(Y, bit_x) (Y|=(1<<bit_x))
#define clr_bit(Y, bit_x) (Y&=~(1<<bit_x))
#define cpl_bit(Y, bit_x) (Y^=(1<<bit_x))

///! Macros Control Variables
/*!
   Buttons  
*/
#define UpBT 5
#define DownBT 4
#define LtBT 7
#define RtBT 6
#define ShootBT 3
#define PauseBT 2

/*!
   Gama Status
*/
#define gamestart 0
#define flag_first 1
#define vshoot 2
#define vbattery 3
#define win 4
#define pause 5
#define flag_up 6

///! Auxiliary Control Variables
/*!
   flags status
*/
char flag = 0x00000010;

/*!
   Game Variables
*/
int points = 0;
float energy_points = 100;
int ship_position[2], asteroid_position[2], energy_position[2], shoot_position[2] = {0, 0};

/*!
   Difficult Variables
*/
int velocity = 5000;
int recpoints = 0;
int dropchance = 0;
int points_to_win = 2;

///! Screen Objects
/*!
   Objects Vectors
*/
byte space_ship[8] ={B11000, B01100, B01110, B01111, B01111, B01110, B01100, B11000};
byte asteroid[8]= {B00000, B00100, B01110, B10111, B11101, B01110, B00100, B00000};
byte explosion[8] = {B10001, B10101, B01010, B10100, B00101, B01010, B10101, B10001};
byte energy[8] = {B01110,B11011,B10001,B10101,B10101,B10101,B10001,B11111};
byte shoot[8] = {B00000,B00000,B00000,B00111,B00111,B00000,B00000,B00000};

LiquidCrystal lcd (13, 12, 11, 10, 9, 8);

ISR(INT0_vect)
{
  cpl_bit(flag, pause);
  set_bit(TIMSK1, OCIE1A);
  
}

ISR(INT1_vect)
{
  set_bit(flag, vshoot);
  shoot_position[0] = (ship_position[0]+1);
  shoot_position[1] = ship_position[1];
}

ISR(TIMER1_COMPA_vect)
{
  lcd.clear(); 
  if(tst_bit(flag, pause))
  {
    clr_bit(TIMSK1, OCIE1A);
   	lcd.setCursor(0, 0);
    lcd.print("GAME PAUSED");
    lcd.setCursor(0, 1);
    lcd.print("PRESS PAUSE BT");
  }
  else
  {
    Draw(asteroid_position, 2);
    Draw(ship_position, 1);
    painel(12);
    
    energy_points -= 0.20;
    
    if (energy_points <= 0)
    {
      animationExplosion(ship_position);
      clr_bit(flag, gamestart);
    }
    if (tst_bit(flag, vshoot))
    {
      Draw(shoot_position, 5);
      shoot_position[0] += 1;		
    }
    
    asteroid_position[0] -= 1;
    
    if (colisionShipAsteroid(ship_position, asteroid_position))
    { 
      for (int i = 0; i<= 50; i++)
      {
      	animationExplosion(ship_position);
      }
      clr_bit(flag, gamestart);
      lcd.clear();
    }

    if((colisionShootAsteroid(shoot_position, asteroid_position)) && (tst_bit(flag, vshoot)))
    {
      clr_bit(flag, vshoot);
      shoot_position[0] = -1;
      animationExplosion(asteroid_position);
      asteroid_position[0] = 12;
      asteroid_position[1] = random(0,2);
      points += 1;
      if (points >= points_to_win)
      {
        set_bit(flag, win);
        clr_bit(flag, gamestart);
        lcd.clear();
      }
    }
    energy_position[0] -= 1;
    
    Draw(energy_position, 4);
    
    if (colisionShootAsteroid(ship_position, energy_position))
    {
      clr_bit(flag, vbattery);
      energy_position[0] = -2;
      energy_points += 40;
    }

  }  
}
void setup()
{
  cli();

  // Configura REGS do PORTD como saída ou entrada. 1 -> OUT, 0-> IN
  DDRD = 0b00000011; 
  PORTD =0b11111100; // Habilita Resistores de Pull-Down Interno
  
  ///! Configuração dos Registradores para Interrupção.
  /*!
    INT 0
  */
  EICRA |=  (1<< ISC01); // Falling Mode
  EIMSK |= (1<<INT0);

  /*!
    INT 1
  */
  EICRA |=  (1<< ISC11); // Falling Mode
  EIMSK |= (1<<INT1);

  /*!
    T1 CTC MODE, 256.
  */
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << WGM12)|(1 << CS12);
  OCR1A = velocity;

  asteroid_position[0] = 12;
  asteroid_position[1] = random(0, 2);

  shoot_position[0] = -1;

  energy_points = 100;
  recpoints = 40;
  dropchance = 50;

  lcd.createChar(1, space_ship);
  lcd.createChar(2, asteroid);
  lcd.createChar(3, explosion);
  lcd.createChar(4, energy);
  lcd.createChar(5, shoot);

  lcd.begin(16,2);
  lcd.clear();
  sei();

}

void loop()
{
  if(tst_bit(flag, gamestart))
  {
    
    // Button Read
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
      set_bit(flag, vshoot);
      shoot_position[0] = (ship_position[0]+1);
      shoot_position[1] = ship_position[1];
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
    
    if (!(tst_bit(flag, vbattery)))
    {
      if(random(0, 101) > dropchance)
      {
        set_bit(flag, vbattery);
        energy_position[0] = 12;
        energy_position[1] = random(0, 2);
      }
    }

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
      set_bit(TIMSK1, OCIE1A);
    }
    else
    {
      if (tst_bit(flag, win))
      {
 		
        clr_bit(TIMSK1, OCIE1A);
        if(!(tst_bit(flag, flag_up)))
        {
          velocity -= 500;
          recpoints -= 10;
          dropchance += 10;
          points_to_win += 2;
        }
        energy_points = 70;
        finalScreen("You win");
        set_bit(flag, flag_up);
      }
      else
      {
        clr_bit(TIMSK1, OCIE1A);
        recpoints = 40 ;
        velocity = 5000;
        dropchance = 50;
        points_to_win = 2;
        energy_points = 100;
        finalScreen("You Lse");
      }
    }

    if(!(tst_bit(PIND, ShootBT)))
    {
      set_bit(TIMSK1, OCIE1A);
      OCR1A = velocity;
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
  clr_bit(flag, flag_up);
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
    \return one interger with value 1 or zero interger with value 0.
*/
int colisionShootAsteroid (int position_ob1[2], int position_ob2[2])
{
    if (((position_ob1[0] == position_ob2[0]+1) && (position_ob1[1] == position_ob2[1])) || ((position_ob1[0] == position_ob2[0]) && (position_ob1[1] == position_ob2[1])))
    {
        return 1;
    }
    else
    {
      	return 0;
    }
}

///! Create Colision Ship With Asteroid.
/*!
    \param position_ob1 array of position with 2 values.
    \param position_ob2 array of position with 2 values.
    \return one interger with value 1 or zero interger with value 0.
*/
int colisionShipAsteroid(int position_ob1[2], int position_ob2[2])
{
    if ((position_ob1[1] == position_ob2[1]) && (position_ob1[0] == position_ob2[0]))
    {
        return 1;
    }
  	else
  	{
      return 0;
  	}
}  
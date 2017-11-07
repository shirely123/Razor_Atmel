/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern u32 G_u32AntApiCurrentMessageTimeStamp;                    /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    /* From ant_api.c */
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  /* From ant_api.c */
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;                /* From ant_api.c */

extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;              /* The state machine function pointer */
static u32 UserApp1_u32Timeout;                        /* Timeout counter used across states */

static AntAssignChannelInfoType UserApp1_sChannelInfo; /* ANT setup parameters */

static u8 UserApp1_au8MessageFail[] = "\n\r***ANT channel setup failed***\n\n\r";

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp1Initialize(void)
{
  u8 au8WelcomeMessage1[] = "Hide and Go Seek!";
  u8 au8WelcomeMessage2[] = "Press B0/B1 to Start";
  
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, au8WelcomeMessage1);
  LCDMessage(LINE2_START_ADDR, au8WelcomeMessage2);
  //u8 au8WelcomeMessage[] = "ANT Master";
  LedOff(PURPLE);
  /* Write a weclome message on the LCD */
#if EIE1
  /* Set a message up on the LCD. Delay is required to let the clear command send. */
  
  for(u32 i = 0; i < 10000; i++);
  //LCDMessage(LINE1_START_ADDR, au8WelcomeMessage);
#endif /* EIE1 */
  
#if 0 // untested for MPG2
  
#endif /* MPG2 */

 /* Configure ANT for this application */
  UserApp1_sChannelInfo.AntChannel          = ANT_CHANNEL_USERAPP;
  UserApp1_sChannelInfo.AntChannelType      = ANT_CHANNEL_TYPE_USERAPP;
  UserApp1_sChannelInfo.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  UserApp1_sChannelInfo.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
 
  UserApp1_sChannelInfo.AntDeviceIdLo       = ANT_DEVICEID_LO_USERAPP;
  UserApp1_sChannelInfo.AntDeviceIdHi       = ANT_DEVICEID_HI_USERAPP;
  UserApp1_sChannelInfo.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  UserApp1_sChannelInfo.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  UserApp1_sChannelInfo.AntFrequency        = ANT_FREQUENCY_USERAPP;
  UserApp1_sChannelInfo.AntTxPower          = ANT_TX_POWER_USERAPP;

  UserApp1_sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    UserApp1_sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
  
  /* Attempt to queue the ANT channel setup */
  if( AntAssignChannel(&UserApp1_sChannelInfo) )
  {
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_AntChannelAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    DebugPrintf(UserApp1_au8MessageFail);
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ANT channel assignment */
static void UserApp1SM_AntChannelAssign()
{
  if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CONFIGURED)
  {
    /* Channel assignment is successful, so open channel and
    proceed to Idle state */
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Watch for time out */
  if(IsTimeUp(&UserApp1_u32Timeout, 3000))
  {
    DebugPrintf(UserApp1_au8MessageFail);
    UserApp1_StateMachine = UserApp1SM_Error;    
  }
     
} /* end UserApp1SM_AntChannelAssign */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */

//Start Seeking
static void UserApp1SM_Seeking(void)
{
  u8 au8UserApp1SM_Seeking[]="Seeker";
  u8 au8UserApp1SM_Seeking1[]="Ready or not";
  u8 au8UserApp1SM_Seeking2[]="Here I come!";
  static u16 u16Counter=10000;
  
  u16Counter--;
  if(u16Counter==9900)
  {
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, au8UserApp1SM_Seeking);
    LCDMessage(LINE2_START_ADDR, "10");
  }
  if(u16Counter==8900)
  {
    LCDMessage(LINE2_START_ADDR, "09");
  }
  if(u16Counter==7900)
  {
    LCDMessage(LINE2_START_ADDR, "08");
  }
  if(u16Counter==6900)
  {
    LCDMessage(LINE2_START_ADDR, "07");
  }
  if(u16Counter==5900)
  {
    LCDMessage(LINE2_START_ADDR, "06");
  }
  if(u16Counter==4900)
  {
    LCDMessage(LINE2_START_ADDR, "05");
  }
  if(u16Counter==3900)
  {
    LCDMessage(LINE2_START_ADDR, "04");
  }
  if(u16Counter==2900)
  {
    LCDMessage(LINE2_START_ADDR, "03");
  }
  if(u16Counter==1900)
  {
    LCDMessage(LINE2_START_ADDR, "02");
  }
  if(u16Counter==900)
  {
    LCDMessage(LINE2_START_ADDR, "01");
  }
  if(u16Counter==0)
  {
    LCDMessage(LINE1_START_ADDR, au8UserApp1SM_Seeking1);
    LCDMessage(LINE2_START_ADDR, au8UserApp1SM_Seeking2);
    UserApp1_StateMachine = UserApp1SM_Seeking_Processing;
  }
}

static void UserApp1SM_Seeking_Processing(void)
{
  static u8 u8Temp;
  u8 au8UserApp1SM_Seeking_Processing[]="Found you!";
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  if( AntReadAppMessageBuffer() )
  {
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      u8Temp = abs (G_sAntApiCurrentMessageExtData.s8RSSI);
      
      //-95 dBm
      if(u8Temp <= 95)
      {
        LedOn(WHITE);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 100);
        au8TestMessage[0] = 0x01;
      }
      else
      {
        LedOff(WHITE);
        PWMAudioOff(BUZZER1);
        au8TestMessage[0] = 0x00;
      }
      //-85dBm
      if(u8Temp <= 85 )
      {
        LedOn(PURPLE);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 200);
        au8TestMessage[1] = 0x01;
      }
      else
      {
        LedOff(PURPLE);
        au8TestMessage[1] = 0x00;
      }
      //-80dBm
      if(u8Temp <= 80)
      {
        LedOn(BLUE);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 300);
        au8TestMessage[2] = 0x01;
      }
      else
      {
        LedOff(BLUE);
        au8TestMessage[2] = 0x00;
      }
      //-75dBm
      if(u8Temp <= 75)
      {
        LedOn(CYAN);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 400);
        au8TestMessage[3] = 0x01;
      }
      else
      {
        LedOff(PURPLE);
        au8TestMessage[3] = 0x00;
      }
      //-70dBm
      if(u8Temp <= 70)
      {
        LedOn(GREEN);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 500);
        au8TestMessage[4] = 0x01;
      }
      else
      {
        LedOff(GREEN);
        au8TestMessage[4] = 0x00;
      }
      //-65dBm
      if(u8Temp <= 65)
      {
        LedOn(YELLOW);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 600);
        au8TestMessage[5] = 0x01;
      }
      else
      {
        LedOff(YELLOW);
        au8TestMessage[5] = 0x00;
      }
      //-55dBm
      if(u8Temp <= 55)
      {
        LedOn(ORANGE);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 700);
        au8TestMessage[6] = 0x01;
      }
      else
      {
        LedOff(YELLOW);
        au8TestMessage[6] = 0x00;
      }
       //-50dBm
      if(u8Temp <= 50)
      {
        LedOn(RED);
        PWMAudioOn(BUZZER1);
        PWMAudioSetFrequency(BUZZER1, 800);
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE1_START_ADDR, au8UserApp1SM_Seeking_Processing);
        au8TestMessage[7] = 0x01;
      }
      else
      {
        LedOff(RED);
        au8TestMessage[7] = 0x00;
      }
      
      AntQueueAcknowledgedMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
    }
    if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {		  	
      if(G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX] == EVENT_TRANSFER_TX_FAILED)
      {
        AntQueueAcknowledgedMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
      }
    }
  }
}

//Start hiding
static void UserApp1SM_Hiding(void)
{
  u8 UserApp1SM_Hiding[] = "Hide!";
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR,UserApp1SM_Hiding);
  UserApp1_StateMachine = UserApp1SM_Hided;	
}

static void UserApp1SM_Hided(void)
{
  u8 UserApp1SM_Hided[] = "You found me!";
  static u8 au8TestMessage2[] = {0, 0, 0, 0, 0, 0, 0, 0};
  
  if( AntReadAppMessageBuffer())
  {
    if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      AntQueueAcknowledgedMessage(ANT_CHANNEL_USERAPP, au8TestMessage2);
    }
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      if(G_au8AntApiCurrentMessageBytes[0] == 0x01 )
      {
        LedOn(WHITE);
      }
      if(G_au8AntApiCurrentMessageBytes[0] == 0x00 )
      {
        LedOff(WHITE);
      }
      if(G_au8AntApiCurrentMessageBytes[1] == 0x01 )
      {
        LedOn(PURPLE);
      }
      if(G_au8AntApiCurrentMessageBytes[1] == 0x00 )
      {
        LedOff(PURPLE);
      }
      if(G_au8AntApiCurrentMessageBytes[2] == 0x01 )
      {
        LedOn(BLUE);
      }
      if(G_au8AntApiCurrentMessageBytes[2] == 0x00 )
      {
        LedOff(BLUE);
      }
      if(G_au8AntApiCurrentMessageBytes[3] == 0x01 )
      {
        LedOn(CYAN);
      }
      if(G_au8AntApiCurrentMessageBytes[3] == 0x00 )
      {
        LedOff(CYAN);
      }
      if(G_au8AntApiCurrentMessageBytes[4] == 0x01 )
      {
        LedOn(GREEN);
      }
      if(G_au8AntApiCurrentMessageBytes[4] == 0x00 )
      {
        LedOff(GREEN);
      }
      if(G_au8AntApiCurrentMessageBytes[5] == 0x01 )
      {
        LedOn(YELLOW);
      }
      if(G_au8AntApiCurrentMessageBytes[5] == 0x00 )
      {
        LedOff(YELLOW);
      }
      if(G_au8AntApiCurrentMessageBytes[6] == 0x01 )
      {
        LedOn(ORANGE);
      }
      if(G_au8AntApiCurrentMessageBytes[6] == 0x00 )
      {
        LedOff(ORANGE);
      }
      if(G_au8AntApiCurrentMessageBytes[7] == 0x01 )
      {
        LedOn(RED);
      }
      if(G_au8AntApiCurrentMessageBytes[7] == 0x00 )
      {
        LedOff(RED);
        LCDCommand(LCD_CLEAR_CMD);
        LCDMessage(LINE2_START_ADDR,UserApp1SM_Hided);
      }
    }
  }
}

static void UserApp1SM_Idle(void)
{
  /* Check all the buttons and update au8TestMessage according to the button state */ 
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);   //???
    UserApp1_StateMachine = UserApp1SM_Seeking;
  }
  if( WasButtonPressed(BUTTON1) )
  {
    ButtonAcknowledge(BUTTON1);
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);   //???
    UserApp1_StateMachine = UserApp1SM_Hiding;
  }
} /* end UserApp1SM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error (for now, do nothing) */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
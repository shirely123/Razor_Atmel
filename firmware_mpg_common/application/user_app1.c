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
    LedBlink(RED, LED_2HZ);
    
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
    LedOff(RED);
    LedOn(YELLOW);
    
    DebugPrintf("ANT Master ready\n\r");
    DebugPrintf("Device ID: ");
    DebugPrintNumber(ANT_DEVICEID_DEC_USERAPP);
    DebugPrintf(", Device Type: ");
    DebugPrintNumber(ANT_DEVICE_TYPE_USERAPP);
    DebugPrintf(", Trans Type: ");
    DebugPrintNumber(ANT_TRANSMISSION_TYPE_USERAPP);
    DebugPrintf(", Frequency: ");
    DebugPrintNumber(ANT_FREQUENCY_USERAPP);
    DebugLineFeed();
    
    /* Channel assignment is successful, so open channel and
    proceed to Idle state */
    AntOpenChannelNumber(ANT_CHANNEL_USERAPP);
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  /* Watch for time out */
  if(IsTimeUp(&UserApp1_u32Timeout, 3000))
  {
    LedBlink(RED, LED_2HZ);
    
    DebugPrintf(UserApp1_au8MessageFail);
    UserApp1_StateMachine = UserApp1SM_Error;    
  }
     
} /* end UserApp1SM_AntChannelAssign */

static void UserAppSM1_WaitChannelClose()
{
  /* Monitor the channel status to check if channel is closed */
  if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CLOSED )
  {
    LedOff(GREEN);
    LedOn(YELLOW);

    UserApp1_StateMachine = UserApp1SM_Idle;
  }

  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, 3000) )
  {
    LedOff(GREEN);
    LedOff(YELLOW);
    LedBlink(RED, LED_2HZ);

    UserApp1_StateMachine = UserApp1SM_Error;
  }
} /* end UserAppSM1_WaitChannelClose() */

static void UserAppSM1_WaitChannelOpen()
{
  /* Monitor the channel status to check if channel is closed */
  if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN )
  {
    LedOn(GREEN);
    LedOff(YELLOW);

    UserApp1_StateMachine = UserApp1SM_Idle;
  }

  /* Check for timeout */
  if( IsTimeUp(&UserApp1_u32Timeout, 3000) )
  {
    LedOff(GREEN);
    LedOn(YELLOW);

    UserApp1_StateMachine = UserApp1SM_Idle;
  }
} /* end UserAppSM1_WaitChannelOpen() */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for ??? */
static void UserApp1SM_Idle(void)
{
  static u8 au8TestMessage[] = {0x5B, 0, 0, 0, 0xFF, 0, 0, 0};
  u8 au8DataContent[26];
  static u8 au8TotalMessage[] = "TOT:                ";
  static u8 au8MissedMessage[] = "MIS:                ";
  
  /* Check the buttons and update au8TestMessage according to the button state */ 
  if( IsButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    
    /*if AntRadioStatusChannel is closed,enter UserAppSM1_WaitChannelOpen*/
    if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_CLOSED )
    {
      LedBlink(GREEN, LED_2HZ);
      
      AntOpenChannelNumber(ANT_CHANNEL_USERAPP);
      
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserAppSM1_WaitChannelOpen;
    }

    /*if AntRadioStatusChannel is open,enter UserAppSM1_WaitChannelClose*/
    if( AntRadioStatusChannel(ANT_CHANNEL_USERAPP) == ANT_OPEN )
    {
      LedBlink(GREEN, LED_2HZ);
      
      AntCloseChannelNumber(ANT_CHANNEL_USERAPP);
      
      UserApp1_u32Timeout = G_u32SystemTime1ms;
      UserApp1_StateMachine = UserAppSM1_WaitChannelClose;
    }
  }
  
  
  if( AntReadAppMessageBuffer() )
  {
     /* New message from ANT task: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      /* We got some data: parse it into au8DataContent[] */
      for(u8 i = 0; i < ANT_DATA_BYTES; i++)
      {
        au8DataContent[3 * i]     = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
        au8DataContent[3 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
        au8DataContent[3 * i + 2] = '-';
      }
      
      au8DataContent[3 * ANT_DATA_BYTES] = '\0';
			
      DebugPrintf(au8DataContent);
      DebugLineFeed();
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      AntQueueAcknowledgedMessage(ANT_CHANNEL_USERAPP, au8TestMessage);
      
      
      /* au8TestMessage 1 to 3 used to count failed message */
      if( G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX] == EVENT_TRANSFER_TX_FAILED )
      {
        au8TestMessage[3]++;
        
        if( au8TestMessage[3] == 0 )
        {
          au8TestMessage[2]++;
          if( au8TestMessage[2] == 0 )
          {
                  au8TestMessage[1]++;
          }
        }
        
        /* get au8MissedMessages and display it on lcd line2 */
        for( u8 i = 0; i < 3; i++ )
        {
          au8MissedMessage[2*i+4] = HexToASCIICharUpper(au8TestMessage[i+1] / 16);
          au8MissedMessage[2*i+5] = HexToASCIICharUpper(au8TestMessage[i+1] % 16);
        }
        
        LCDMessage(LINE2_START_ADDR,au8MissedMessage);
      }
      
      /* au8TestMessage 5 to 7 used to count total message */
      if( G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX] == EVENT_TRANSFER_TX_COMPLETED )
      {
        au8TestMessage[7]++;
        
        if( au8TestMessage[7] == 0 )
        {
          au8TestMessage[6]++;
          if( au8TestMessage[6] == 0 )
          {
            au8TestMessage[5]++;
          }
        }
        
        /* get au8TotalMessage1 and display it on lcd line1 */
        for( u8 i = 0; i < 3; i++ )
        {
          au8TotalMessage[2*i+4] = HexToASCIICharUpper(au8TestMessage[i+5] / 16);
          au8TotalMessage[2*i+5] = HexToASCIICharUpper(au8TestMessage[i+5] % 16);
        }
        
        LCDMessage(LINE1_START_ADDR,au8TotalMessage);
      }
    }
  } /* end AntReadData() */
  
} /* end UserApp1SM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error (for now, do nothing) */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

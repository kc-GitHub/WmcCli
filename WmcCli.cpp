/***********************************************************************************************************************
   @file  WmcCli.cpp
   @brief Simple command line interface for the WMC app.
 **********************************************************************************************************************/

/***********************************************************************************************************************
   I N C L U D E S
 **********************************************************************************************************************/
#include "WmcCli.h"
#include "LoclibData.h"
#include "app_cfg.h"
#include "eep_cfg.h"
#include "fsmlist.hpp"
#include <EEPROM.h>
#include <stdio.h>

#include <WiFiManager.h>            //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

/***********************************************************************************************************************
   D A T A   D E C L A R A T I O N S (exported, local)
 **********************************************************************************************************************/
const char* WmcCli::LocAdd       = "add";
const char* WmcCli::LocDelete    = "del ";
const char* WmcCli::LocChange    = "change ";
const char* WmcCli::LocName      = "name ";
const char* WmcCli::LocDeleteAll = "clear";
const char* WmcCli::EraseAll     = "erase";
const char* WmcCli::Emergency    = "emergency ";
const char* WmcCli::Help         = "help";
const char* WmcCli::LocList      = "list";
const char* WmcCli::Ac           = "ac";
const char* WmcCli::Dump         = "dump";
const char* WmcCli::Settings     = "settings";
const char* WmcCli::Reset        = "reset";
#if APP_CFG_UC == APP_CFG_UC_ESP8266
const char* WmcCli::Ssid          = "ssid ";
const char* WmcCli::IpAdrressZ21  = "z21";
const char* WmcCli::Network       = "network";
#endif

/***********************************************************************************************************************
   F U N C T I O N S
 **********************************************************************************************************************/

/***********************************************************************************************************************
 */
WmcCli::WmcCli()
{
    m_bufferRxIndex = 0;
    m_Address       = 0;
    m_DecoderSteps  = 0;
    m_Function      = 0;
    m_Button        = 0;
}

/***********************************************************************************************************************
 */
void WmcCli::Init(LocLib LocLib, LocStorage LocStorage)
{
    Serial.begin(76800);
    m_locLib     = LocLib;
    m_LocStorage = LocStorage;
}

/***********************************************************************************************************************
 * Read data from serial port, if CR is received check the received data and perform action if valid command is
 * received.
 */
void WmcCli::Update(void)
{
    int DataRx;

    DataRx = Serial.read();

    while (DataRx != -1)
    {
        Serial.print((char)(DataRx));
        switch (DataRx)
        {
        case 0x0A: break;
        case 0x0D:
            Process();
            m_bufferRxIndex = 0;
            memset(m_bufferRx, '\0', sizeof(m_bufferRx));
            break;
        default:
            m_bufferRx[m_bufferRxIndex] = (char)(DataRx);
            m_bufferRxIndex++;

            if (m_bufferRxIndex >= sizeof(m_bufferRx))
            {
                m_bufferRxIndex = 0;
                memset(m_bufferRx, '\0', sizeof(m_bufferRx));
            }
            break;
        }

        DataRx = Serial.read();
    }
}

/***********************************************************************************************************************
 */
void WmcCli::Process(void)
{
    if (strncmp(m_bufferRx, Help, strlen(Help)) == 0)
    {
        HelpScreen();
    }
    else if (strncmp(m_bufferRx, LocAdd, strlen(LocAdd)) == 0)
    {
        if (Add() == true)
        {
            send_event(Event);
        }
    }
    else if (strncmp(m_bufferRx, LocDelete, strlen(LocDelete)) == 0)
    {
        if (Delete() == true)
        {
            send_event(Event);
        }
    }
    else if (strncmp(m_bufferRx, LocDeleteAll, strlen(LocDeleteAll)) == 0)
    {
        m_locLib.InitialLocStore();
        Serial.println("All locs cleared.");
        send_event(Event);
    }
    else if (strncmp(m_bufferRx, EraseAll, strlen(EraseAll)) == 0)
    {
        m_locLib.InitialLocStore();
        m_LocStorage.AcOptionSet(0);
        m_LocStorage.EmergencyOptionSet(0);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
        WiFiManager wifiManager;
        wifiManager.resetSettings();
#elif APP_CFG_UC == APP_CFG_UC_STM32
        m_LocStorage.XpNetAddressSet(255);
#endif

        Serial.println("All data cleared.");
        send_event(Event);
    }
    else if (strncmp(m_bufferRx, Emergency, strlen(Emergency)) == 0)
    {
        if (EmergencyChange())
        {
            send_event(Event);
        }
    }
    else if (strncmp(m_bufferRx, LocChange, strlen(LocChange)) == 0)
    {
        if (Change() == true)
        {
            send_event(Event);
        }
    }
    else if (strncmp(m_bufferRx, LocName, strlen(LocName)) == 0)
    {
        if (SetName() == true)
        {
            send_event(Event);
        }
    }

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    else if (strncmp(m_bufferRx, Network, strlen(Network)) == 0)
    {
        ShowNetworkSettings();
    }
#endif
    else if (strncmp(m_bufferRx, LocList, strlen(LocList)) == 0)
    {
        ListAllLocs();
    }
    else if (strncmp(m_bufferRx, Dump, strlen(Dump)) == 0)
    {
        DumpData();
    }
    else if (strncmp(m_bufferRx, Settings, strlen(Settings)) == 0)
    {
        ShowSettings();
    }
    else if (strncmp(m_bufferRx, Ac, strlen(Ac)) == 0)
    {
        if (AcControlType() == true)
        {
            send_event(Event);
        }
    }
#if APP_CFG_UC == APP_CFG_UC_STM32
    else if (strncmp(m_bufferRx, Reset, strlen(Reset)) == 0)
    {
        nvic_sys_reset();
    }
#endif
    else
    {
        Serial.println("Unknown command.");
    }
}

/***********************************************************************************************************************
 */
void WmcCli::HelpScreen(void)
{
    Serial.println("add x           : Add loc with address x.");
    Serial.println("name x y        : Set name of loc address x with name y.");
    Serial.println("del x           : Delete loc with address x.");
    Serial.println("clear           : Delete ALL locs.");
    Serial.println("erase           : Erase ALL data.");
    Serial.println("change x y z    : Assign function z to button y of loc with address x.");
    Serial.println("emergency x     : Set power off (0) or emergency stop (1).");
    Serial.println("list            : Show all programmed locs.");
    Serial.println("dump            : Dump data for backup.");
    Serial.println("ac x            : Enable (x=1) / disable (x=0) AC control option.");
    Serial.println("settings        : Show overview of settings.");
#if APP_CFG_UC == APP_CFG_UC_STM32
    Serial.println("reset           : Perform reset.");
#endif
    Serial.println("help            : This screen.");
}

/***********************************************************************************************************************
 */
#if APP_CFG_UC == APP_CFG_UC_ESP8266
/***********************************************************************************************************************
 */
void WmcCli::ShowNetworkSettings(void)
{
    memset(m_IpAddressZ21, 0, sizeof(IpAdrressZ21));

    /* Get and print the network settings. */
    Serial.print(Ssid);
    Serial.println(WiFi.SSID());

    EEPROM.get(EepCfg::EepIpAddressZ21, m_IpAddressZ21);
    IpDataPrint(IpAdrressZ21, m_IpAddressZ21);
}
#endif

/***********************************************************************************************************************
 */
bool WmcCli::Add(void)
{
    uint8_t Functions[5] = { 0, 1, 2, 3, 4 };
    bool Result          = false;

    m_Address = atoi(&m_bufferRx[strlen(LocAdd)]);

    if ((m_Address > 0) && (m_Address <= 9999))
    {
        /* Add loc, default functions 0..4 */
        if (m_locLib.StoreLoc(m_Address, Functions, NULL, LocLib::storeAdd) == true)
        {
            m_locLib.LocBubbleSort();
            Serial.print("Loc with address ");
            Serial.print(m_Address);
            Serial.println(" added.");
            Result = true;
        }
        else
        {
            Serial.println("Loc add failed, loc already present!");
        }
    }
    else
    {
        Serial.println("Loc add command not ok.!");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
bool WmcCli::Delete(void)
{
    m_Address   = atoi(&m_bufferRx[strlen(LocDelete)]);
    bool Result = false;

    if (m_locLib.RemoveLoc(m_Address) == true)
    {
        Serial.print("Loc ");
        Serial.print(m_Address);
        Serial.println(" deleted.");
        Result = true;
    }
    else
    {
        Serial.println("Loc delete failed!");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
bool WmcCli::Change(void)
{
    char* Space;
    uint8_t FunctionAssignment[5];
    bool Result = false;

    Space = strchr(m_bufferRx, 32);
    Space++;

    m_Address = atoi(Space);

    Space = strchr(Space, 32);

    if (Space != NULL)
    {
        Space++;
        m_Button = atoi(Space);

        Space = strchr(Space, 32);
        if (Space != NULL)
        {
            Space++;
            m_Function = atoi(Space);

            /* Get actual assigned functions of loc. */
            if (m_locLib.CheckLoc(m_Address) != 255)
            {
                if (m_Button < 5)
                {
                    if (m_Function < 29)
                    {
                        m_locLib.FunctionAssignedGetStored(m_Address, FunctionAssignment);
                        FunctionAssignment[m_Button] = m_Function;
                        m_locLib.StoreLoc(m_Address, FunctionAssignment, NULL, LocLib::storeChange);
                        Serial.println("Loc function updated.");
                        Result = true;
                    }
                    else
                    {
                        Serial.println("Invalid function number, must be 0..28");
                    }
                }
                else
                {
                    Serial.println("Invalid button number, must be 0..4");
                }
            }
            else
            {
                Serial.print("Loc ");
                Serial.print(m_Address);
                Serial.println(" is not present.");
            }
        }
        else
        {
            Serial.println("Command invalid.");
        }
    }
    else
    {
        Serial.println("Command invalid.");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
bool WmcCli::SetName(void)
{
    char* Space;
    bool Result = false;

    Space = strchr(m_bufferRx, 32);
    Space++;

    m_Address = atoi(Space);

    Space = strchr(Space, 32);

    if (Space != NULL)
    {
        Space++;
        /* Get data of loc. */
        if (m_locLib.CheckLoc(m_Address) != 255)
        {
            m_locLib.StoreLoc(m_Address, NULL, Space, LocLib::storeChange);
            Serial.println("Loc name updated.");
            Result = true;
        }
        else
        {
            Serial.print("Loc ");
            Serial.print(m_Address);
            Serial.println(" is not present.");
        }
    }
    else
    {
        Serial.println("Command invalid.");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
void WmcCli::ListAllLocs(void)
{
    uint8_t Index = 0;
    LocLibData* Data;
    char output[30];

    /* Print header. */
    Serial.println("          Functions                         Functions               ");
    Serial.println("Address B0 B1 B2 B3 B4  Name      Address B0 B1 B2 B3 B4  Name      ");

    /* Print two locs with info on one line. */
    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);
        sprintf(output, "%4hu    %2hu %2hu %2hu %2hu %2hu %-8s", Data->Addres, Data->FunctionAssignment[0],
            Data->FunctionAssignment[1], Data->FunctionAssignment[2], Data->FunctionAssignment[3],
            Data->FunctionAssignment[4], Data->Name);
        Serial.print(output);

        Index++;
        if ((Index % 2) == 0)
        {
            Serial.println();
        }
    }

    Serial.println();
}

/***********************************************************************************************************************
 */
bool WmcCli::EmergencyChange(void)
{
    char* Space;
    uint8_t EmergencyOption;
    bool Result = false;

    Space = strchr(m_bufferRx, 32);

    if (Space != NULL)
    {
        Space++;

        EmergencyOption = atoi(Space);

        switch (EmergencyOption)
        {
        case 0:
            m_LocStorage.EmergencyOptionSet(0);
            Serial.println("Stop option set to power off.");
            Result = true;
            break;
        case 1:
            m_LocStorage.EmergencyOptionSet(1);
            Serial.println("Stop option set to emergency stop");
            Result = true;
            break;
        default:
            Serial.println("Emergency entry invalid, set to power off.");
            m_LocStorage.EmergencyOptionSet(0);
            break;
        }
    }
    else
    {
        Serial.println("Emergency option entry invalid, must be emergency 0 or emergency 1");
    }

    return (Result);
}
/***********************************************************************************************************************
 */
bool WmcCli::AcControlType(void)
{
    uint8_t AcOption;
    char* Space;
    bool Result = false;

    Space = strchr(m_bufferRx, 32);

    if (Space != NULL)
    {
        Space++;

        AcOption = atoi(Space);

        switch (AcOption)
        {
        case 0:
            m_LocStorage.AcOptionSet(AcOption);
            Serial.println("AC option disabled.");
            Result = true;
            break;
        case 1:
            m_LocStorage.AcOptionSet(AcOption);
            Serial.println("AC option enabled.");
            Result = true;
            break;
        default:
            Serial.println("AC option entry invalid, option set to disabled.");
            m_LocStorage.AcOptionSet(0);
            break;
        }
    }
    else
    {
        Serial.println("AC option entry invalid, must be ac 0 or ac 1");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
void WmcCli::DumpData(void)
{
    uint8_t Index          = 0;
    uint8_t FunctionIndex  = 0;
    uint16_t AcOption      = 0;
    uint16_t EmergencyStop = 0;
    LocLibData* Data       = NULL;

    // Loc address and functions
    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);
        Serial.print(LocAdd);
        Serial.println(Data->Addres);

        for (FunctionIndex = 0; FunctionIndex < 5; FunctionIndex++)
        {
            Serial.print(LocChange);
            Serial.print(Data->Addres);
            Serial.print(" ");
            Serial.print(FunctionIndex);
            Serial.print(" ");
            Serial.println(Data->FunctionAssignment[FunctionIndex]);
        }

        Index++;
    }

    // Locnames
    Index = 0;
    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);
        if (strlen(Data->Name) > 0)
        {
            Serial.print(LocName);
            Serial.print(Data->Addres);
            Serial.print(" ");
            Serial.println(Data->Name);
        }
        Index++;
    }

    /* Dump the AC option. */
    Serial.print(Ac);
    Serial.print(" ");
    AcOption = EEPROM.read(EepCfg::AcTypeControlAddress);
    if (AcOption > 1)
    {
        AcOption = 0;
    }
    Serial.println(AcOption);

    /* Dump the emergency option. */
    Serial.print(Emergency);
    Serial.print(" ");
    EmergencyStop = EEPROM.read(EepCfg::EmergencyStopEnabledAddress);
    if (EmergencyStop > 1)
    {
        EmergencyStop = 0;
    }
    Serial.println(EmergencyStop);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    ShowNetworkSettings();
#endif
}

/***********************************************************************************************************************
 */
void WmcCli::ShowSettings(void)
{
    Serial.print("Number of locs  : ");
    Serial.println(m_locLib.GetNumberOfLocs());

    Serial.print("Ac control      : ");
    if (m_LocStorage.AcOptionGet() == 1)
    {
        Serial.println("On.");
    }
    else
    {
        Serial.println("Off.");
    }
    Serial.print("Emergency stop  : ");
    if (m_LocStorage.EmergencyOptionGet() == 1)
    {
        Serial.println("Enabled.");
    }
    else
    {
        Serial.println("Disabled.");
    }

#if APP_CFG_UC == APP_CFG_UC_STM32
    Serial.print("XPessNet address: ");
    Serial.println(m_LocStorage.XpNetAddressGet());
#else
    /* Get and print the network settings. */
    Serial.print("Ssid            : ");
    Serial.println(WiFi.SSID());

    EEPROM.get(EepCfg::EepIpAddressZ21, m_IpAddressZ21);
    IpDataPrint("Ip address Z21  : ", m_IpAddressZ21);
#endif
}

#if APP_CFG_UC == APP_CFG_UC_ESP8266
/***********************************************************************************************************************
 */
void WmcCli::IpDataPrint(const char* StrPtr, uint8_t* IpDataPtr)
{
    Serial.print(StrPtr);
    Serial.print(IpDataPtr[0]);
    Serial.print(".");
    Serial.print(IpDataPtr[1]);
    Serial.print(".");
    Serial.print(IpDataPtr[2]);
    Serial.print(".");
    Serial.println(IpDataPtr[3]);
}
#endif

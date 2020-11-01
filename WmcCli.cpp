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

#if APP_CFG_UC == APP_CFG_UC_ESP8266
#include <WiFiManager.h>            //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#endif

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
const char* WmcCli::Exit          = "exit";
const char* WmcCli::IpAdrressZ21  = "z21 ";
const char* WmcCli::Network       = "network";
ESPTelnet telnet;
#endif

/***********************************************************************************************************************
   F U N C T I O N S
 **********************************************************************************************************************/
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    WmcCli *wmcClassPointer; // declare a pointer to testLib class

    /**
     * Callback was called from telnet class at connect.
     */
    static void onTelnetConnect()
    {
        Serial.println("Telnet: " + telnet.getIP() + " connected");

        telnet.println("Welcome " + telnet.getIP() + "\n");
        telnet.println("Type \"exit\" to disconnect.\n");
    }

    /**
     * Callback was called from telnet if data available
     */
    static void onDataAvailable(void)
    {
        String DataRx = telnet.getData();
        wmcClassPointer->Process(const_cast<char*>(DataRx.c_str()));
    }
#endif

/***********************************************************************************************************************
 */
WmcCli::WmcCli()
{
    m_bufferRxIndex = 0;
    m_Address       = 0;
    m_DecoderSteps  = 0;
    m_Function      = 0;
    m_Button        = 0;

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    wmcClassPointer = this;
#endif
}

size_t WmcCli::print(const char str[]) {
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.print((String)str);
        return 0;
    #else
        return Serial.print(str);
    #endif
}

size_t WmcCli::print(const String &s) {
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.print(s.c_str());
        return 0;
    #else
        return Serial.print(s.c_str());
    #endif
}

size_t WmcCli::print(int n, int base) {
    UNUSED(base);
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.print((String)n);
        return 0;
    #else
        return Serial.print((long) n, base);
    #endif
}

size_t WmcCli::print(unsigned char b, int base) {
    UNUSED(base);
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.print((String)b);
        return 0;
    #else
        return Serial.print((long) b, base);
    #endif
}

size_t WmcCli::println(unsigned char b, int base) {
    size_t n = print(b, base);
    n += println();
    return n;
}

size_t WmcCli::println(int num, int base) {
    size_t n = print(num, base);
    n += println();
    return n;
}

size_t WmcCli::println(const String &s) {
    size_t n = print(s);
    n += println();
    return n;
}

size_t WmcCli::println(void) {
    return print("\r\n");
}

/***********************************************************************************************************************
 */
void WmcCli::Init(LocLib LocLib, LocStorage LocStorage)
{
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        Serial.begin(76800);

        if (telnet.begin()) {
          telnet.onConnect(onTelnetConnect);
          telnet.onDataAvailable(onDataAvailable);
          Serial.println("Telnet started.");

        } else {
          Serial.println("Failed to start Telnet server. Is WiFi connected?");
        }
    #else
        Serial.begin(76800);
    #endif

    m_locLib     = LocLib;
    m_LocStorage = LocStorage;
}

/***********************************************************************************************************************
 * Read data from serial port, if CR is received check the received data and perform action if valid command is
 * received.
 */
void WmcCli::Update(void)
{
    #if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.loop();
    #else
        int DataRx = Serial.read();

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
    #endif
}

/***********************************************************************************************************************
 */

void WmcCli::Process(char *bufferRx) {
    strcpy(m_bufferRx, bufferRx);
    Process();
}

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
        println("All locs cleared.");
        send_event(Event);
    }
    else if (strncmp(m_bufferRx, EraseAll, strlen(EraseAll)) == 0)
    {
        m_locLib.InitialLocStore();
        m_LocStorage.AcOptionSet(0);
        m_LocStorage.EmergencyOptionSet(0);
        println("All data cleared.");

#if APP_CFG_UC == APP_CFG_UC_ESP8266
        telnet.println("Bye");
        delay(1000);
        WiFiManager wifiManager;
        wifiManager.resetSettings();
#elif APP_CFG_UC == APP_CFG_UC_STM32
        m_LocStorage.XpNetAddressSet(255);
#endif

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
    else if (strncmp(m_bufferRx, Exit, strlen(Network)) == 0)
    {
        telnet.println("Bye");
        telnet.close();
    }
    else if (strncmp(m_bufferRx, IpAdrressZ21, strlen(IpAdrressZ21)) == 0)
    {
        if (IpAddressWriteZ21() == true)
        {
            send_event(Event);
        }
    }
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
        println("Unknown command. " + String(m_bufferRx));
    }
}

/***********************************************************************************************************************
 */
void WmcCli::HelpScreen(void)
{
    println("add x           : Add loc with address x.");
    println("name x y        : Set name of loc address x with name y.");
    println("del x           : Delete loc with address x.");
    println("clear           : Delete ALL locs.");
    println("erase           : Erase ALL data.");
    println("change x y z    : Assign function z to button y of loc with address x.");
    println("emergency x     : Set power off (0) or emergency stop (1).");
    println("list            : Show all programmed locs.");
    println("dump            : Dump data for backup.");
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    println("z21 a.b.c.d     : Set IP address of Z21 control.");
#endif
    println("ac x            : Enable (x=1) / disable (x=0) AC control option.");
    println("settings        : Show overview of settings.");
#if APP_CFG_UC == APP_CFG_UC_STM32
    println("reset           : Perform reset.");
#else
    println("network         : Show network settings");
#endif
    println("help            : This screen.");
}

/***********************************************************************************************************************
 */
#if APP_CFG_UC == APP_CFG_UC_ESP8266
/***********************************************************************************************************************
 */
bool WmcCli::IpAddressWriteZ21(void)
{
    bool Result = true;

    if (IpGetData(IpAdrressZ21, m_IpAddressZ21) == true)
    {
        EEPROM.put(EepCfg::EepIpAddressZ21, m_IpAddressZ21);
        EEPROM.commit();

        print("IP Address Z21 stored : ");
        print(m_IpAddressZ21[0]);
        print(".");
        print(m_IpAddressZ21[1]);
        print(".");
        print(m_IpAddressZ21[2]);
        print(".");
        println(m_IpAddressZ21[3]);
    }
    else
    {
        Result = false;
        println("IP Address Z21 entry invalid!");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
void WmcCli::ShowNetworkSettings(void)
{
    /* Get and print the network settings. */
    print("SSID            : ");
    println(WiFi.SSID());

    print("IP address      : ");
    println(WiFi.localIP().toString());

    memset(m_IpAddressZ21, 0, sizeof(IpAdrressZ21));
    EEPROM.get(EepCfg::EepIpAddressZ21, m_IpAddressZ21);
    IpDataPrint("IP address Z21  : ", m_IpAddressZ21);
}
#endif

/***********************************************************************************************************************
 */
bool WmcCli::Add(void)
{
    uint8_t Functions[MAX_FUNCTION_BUTTONS];
    for (uint8_t i = 0; i < MAX_FUNCTION_BUTTONS; i++)
    {
        Functions[i] = i;
    }

    bool Result          = false;

    m_Address = atoi(&m_bufferRx[strlen(LocAdd)]);

    if ((m_Address > 0) && (m_Address <= 9999))
    {
        /* Add loc, default functions 0..4 */
        if (m_locLib.StoreLoc(m_Address, Functions, NULL, LocLib::storeAdd) == true)
        {
            m_locLib.LocBubbleSort();
            print("Loc with address ");
            print(m_Address);
            println(" added.");
            Result = true;
        }
        else
        {
            println("Loc add failed, loc already present!");
        }
    }
    else
    {
        println("Loc add command not ok.!");
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
        print("Loc ");
        print(m_Address);
        println(" deleted.");
        Result = true;
    }
    else
    {
        println("Loc delete failed!");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
bool WmcCli::Change(void)
{
    char* Space;
    uint8_t FunctionAssignment[MAX_FUNCTION_BUTTONS];
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
                if (m_Button < MAX_FUNCTION_BUTTONS)
                {
                    if (m_Function < 29)
                    {
                        m_locLib.FunctionAssignedGetStored(m_Address, FunctionAssignment);
                        FunctionAssignment[m_Button] = m_Function;
                        m_locLib.StoreLoc(m_Address, FunctionAssignment, NULL, LocLib::storeChange);
                        println("Loc function updated.");
                        Result = true;
                    }
                    else
                    {
                        println("Invalid function number, must be 0..28");
                    }
                }
                else
                {
                    uint8_t maxFunctionButtons = MAX_FUNCTION_BUTTONS - 1;
                    println("Invalid button number, must be 0.." + (String)maxFunctionButtons);
                }
            }
            else
            {
                print("Loc ");
                print(m_Address);
                println(" is not present.");
            }
        }
        else
        {
            println("Command invalid.");
        }
    }
    else
    {
        println("Command invalid.");
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
            println("Loc name updated.");
            Result = true;
        }
        else
        {
            print("Loc ");
            print(m_Address);
            println(" is not present.");
        }
    }
    else
    {
        println("Command invalid.");
    }

    return (Result);
}

/***********************************************************************************************************************
 */
void WmcCli::ListAllLocs(void)
{
    uint8_t Index = 0;
    LocLibData* Data;
    char output[52];
    uint8_t i = 0;

    /* Print header. */

    String space = "        ";
    for (i = 0; i < MAX_FUNCTION_BUTTONS; i++)
    {
        space += "   ";
    }
    println("        Functions" + space + "    Functions" + space);

    String buttons = "";
    for (i = 0; i < MAX_FUNCTION_BUTTONS; i++)
    {
        buttons += " B" + (String)i;
    }
    println("Address" + buttons + "  Name        Address" + buttons + "  Name      ");

    /* Print two locs with info on one line. */
    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);

        buttons = "";
        for (i = 0; i < MAX_FUNCTION_BUTTONS; i++)
        {
            sprintf(output, "%2hu ", Data->FunctionAssignment[i]);
            buttons += (String)output;
        }

        sprintf(output, "%7hu %s %-10s  ", Data->Addres, buttons.c_str(), Data->Name);
        print(output);

        Index++;
        if ((Index % 2) == 0)
        {
            println();
        }
    }

    println();
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
            println("Stop option set to power off.");
            Result = true;
            break;
        case 1:
            m_LocStorage.EmergencyOptionSet(1);
            println("Stop option set to emergency stop");
            Result = true;
            break;
        default:
            println("Emergency entry invalid, set to power off.");
            m_LocStorage.EmergencyOptionSet(0);
            break;
        }
    }
    else
    {
        println("Emergency option entry invalid, must be emergency 0 or emergency 1");
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
            println("AC option disabled.");
            Result = true;
            break;
        case 1:
            m_LocStorage.AcOptionSet(AcOption);
            println("AC option enabled.");
            Result = true;
            break;
        default:
            println("AC option entry invalid, option set to disabled.");
            m_LocStorage.AcOptionSet(0);
            break;
        }
    }
    else
    {
        println("AC option entry invalid, must be ac 0 or ac 1");
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
        print(LocAdd);
        print(" ");
        println(Data->Addres);

        for (FunctionIndex = 0; FunctionIndex < MAX_FUNCTION_BUTTONS; FunctionIndex++)
        {
            print(LocChange);
            print(Data->Addres);
            print(" ");
            print(FunctionIndex);
            print(" ");
            println(Data->FunctionAssignment[FunctionIndex]);
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
            print(LocName);
            print(Data->Addres);
            print(" ");
            println(Data->Name);
        }
        Index++;
    }

    /* Dump the AC option. */
    print(Ac);
    print(" ");
    AcOption = EEPROM.read(EepCfg::AcTypeControlAddress);
    if (AcOption > 1)
    {
        AcOption = 0;
    }
    println(AcOption);

    /* Dump the emergency option. */
    print(Emergency);
    print(" ");
    EmergencyStop = EEPROM.read(EepCfg::EmergencyStopEnabledAddress);
    if (EmergencyStop > 1)
    {
        EmergencyStop = 0;
    }
    println(EmergencyStop);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    memset(m_IpAddressZ21, 0, sizeof(IpAdrressZ21));
    EEPROM.get(EepCfg::EepIpAddressZ21, m_IpAddressZ21);
    IpDataPrint(IpAdrressZ21, m_IpAddressZ21);
#endif
}

/***********************************************************************************************************************
 */
void WmcCli::ShowSettings(void)
{
    print("Number of locs  : ");
    println(m_locLib.GetNumberOfLocs());

    print("Ac control      : ");
    if (m_LocStorage.AcOptionGet() == 1)
    {
        println("On.");
    }
    else
    {
        println("Off.");
    }
    print("Emergency stop  : ");
    if (m_LocStorage.EmergencyOptionGet() == 1)
    {
        println("Enabled.");
    }
    else
    {
        println("Disabled.");
    }

#if APP_CFG_UC == APP_CFG_UC_STM32
    print("XPessNet address: ");
    println(m_LocStorage.XpNetAddressGet());
#else
    ShowNetworkSettings();
#endif
}

#if APP_CFG_UC == APP_CFG_UC_ESP8266
/***********************************************************************************************************************
 */

/***********************************************************************************************************************
 */
bool WmcCli::IpGetData(const char* SourcePtr, uint8_t* TargetPtr)
{
    char* Dot;
    bool Result   = true;
    uint8_t Index = 0;

    /* s(s)canf is not present, so get digits by locating the dot and getting value from the dot location with
     * atoi function. */

    TargetPtr[0] = atoi(&m_bufferRx[strlen(SourcePtr)]);
    Dot          = &m_bufferRx[strlen(SourcePtr)];

    while ((Index < 3) && (Dot != NULL))
    {
        Dot = strchr(Dot, '.');
        if (Dot != NULL)
        {
            Dot++;
            TargetPtr[1 + Index] = atoi(Dot);
            Index++;
        }
        else
        {
            Result = false;
        }
    }

    return (Result);
}

void WmcCli::IpDataPrint(const char* StrPtr, uint8_t* IpDataPtr)
{
    print(StrPtr);
    print(IpDataPtr[0]);
    print(".");
    print(IpDataPtr[1]);
    print(".");
    print(IpDataPtr[2]);
    print(".");
    println(IpDataPtr[3]);
}
#endif

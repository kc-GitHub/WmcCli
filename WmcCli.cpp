/***********************************************************************************************************************
   @file  WmcCli.cpp
   @brief Simple command line interface for the WMC app.
 **********************************************************************************************************************/

/***********************************************************************************************************************
   I N C L U D E S
 **********************************************************************************************************************/
#include "WmcCli.h"
#include "eep_cfg.h"
#include <EEPROM.h>
#include <stdio.h>

/***********************************************************************************************************************
   D A T A   D E C L A R A T I O N S (exported, local)
 **********************************************************************************************************************/
const char* WmcCli::LocAdd    = "add";
const char* WmcCli::LocDelete = "del ";
const char* WmcCli::LocChange = "change ";
const char* WmcCli::Ssid      = "ssid ";
const char* WmcCli::Password  = "password ";
const char* WmcCli::IpAdrress = "ip ";
const char* WmcCli::Network   = "network";
const char* WmcCli::Help      = "help";
const char* WmcCli::LocList   = "list";
const char* WmcCli::Ac        = "ac";
const char* WmcCli::Dump      = "dump";

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
void WmcCli::Init()
{
    Serial.begin(115200);
    m_locLib.Init();
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
        Add();
    }
    else if (strncmp(m_bufferRx, LocDelete, strlen(LocDelete)) == 0)
    {
        Delete();
    }
    else if (strncmp(m_bufferRx, LocChange, strlen(LocChange)) == 0)
    {
        Change();
    }
    else if (strncmp(m_bufferRx, Ssid, strlen(Ssid)) == 0)
    {
        // Write SSID name to EEPROM
        SsIdWriteName();
    }
    else if (strncmp(m_bufferRx, Password, strlen(Password)) == 0)
    {
        // Write SSID password to EEPROM
        SsIdWritePassword();
    }
    else if (strncmp(m_bufferRx, IpAdrress, strlen(IpAdrress)) == 0)
    {
        // Write ip address to connect to.
        IpAddressWrite();
    }
    else if (strncmp(m_bufferRx, Network, strlen(Network)) == 0)
    {
        // Write ip address to connect to.
        ShowNetworkSettings();
    }
    else if (strncmp(m_bufferRx, LocList, strlen(LocList)) == 0)
    {
        ListAllLocs();
    }
    else if (strncmp(m_bufferRx, Dump, strlen(Dump)) == 0)
    {
        DumpData();
    }
    else if (strncmp(m_bufferRx, Ac, strlen(Ac)) == 0)
    {
        AcControlType();
    }
    else
    {
        Serial.println("Unknown command.");
    }
}

/***********************************************************************************************************************
 */
void WmcCli::HelpScreen(void)
{
    Serial.println("add x        : Add loc with address x");
    Serial.println("del x        : Delete loc with address x");
    Serial.println("change x y z : Assign function z to button y of loc with address x.");
    Serial.println("list         : Show all programmed locs.");
    Serial.println("dump         : Dump data for backup.");
    Serial.println("ssid <>      : Set SSID name (Wifi) to connect to.");
    Serial.println("password <>  : Set password (Wifi).");
    Serial.println("ip a.b.c.d   : Set IP address of Z21 control.");
    Serial.println("network      : Show programmed IP and network settings.");
    Serial.println("ac           : Enable / disable AC control option.");
    Serial.println("help         : This screen.");
}

/***********************************************************************************************************************
 */
void WmcCli::SsIdWriteName(void)
{
    memset(m_SsidName, '\0', sizeof(m_SsidName));
    memcpy(m_SsidName, &m_bufferRx[strlen(Ssid)], strlen(&m_bufferRx[strlen(Ssid)]));
    EEPROM.put(EepCfg::SsidAddress, m_SsidName);
    EEPROM.commit();

    Serial.print("SSID name : ");
    Serial.print(m_SsidName);
    Serial.println(" stored.");
}

/***********************************************************************************************************************
 */
void WmcCli::SsIdWritePassword(void)
{
    memset(m_SsidPassword, '\0', sizeof(m_SsidPassword));
    memcpy(m_SsidPassword, &m_bufferRx[strlen(Password)], strlen(&m_bufferRx[strlen(Password)]));
    EEPROM.put(EepCfg::SsidPasswordAddress, m_SsidPassword);
    EEPROM.commit();

    Serial.print("SSID password : ");
    Serial.print(m_SsidPassword);
    Serial.println(" stored.");
}

/***********************************************************************************************************************
 */
void WmcCli::IpAddressWrite(void)
{
    char* Dot;
    uint8_t Index;

    /* s(s)canf not present, so get digits by locating the dot and getting value from the dot location. */
    m_IpAddress[0] = atoi(&m_bufferRx[strlen(IpAdrress)]);
    Dot            = &m_bufferRx[strlen(IpAdrress)];

    for (Index = 0; Index < 3; Index++)
    {
        Dot = strchr(Dot, '.');
        Dot++;
        m_IpAddress[1 + Index] = atoi(Dot);
    }

    EEPROM.put(EepCfg::EepIpAddress, m_IpAddress);
    EEPROM.commit();

    Serial.print("IP Address stored : ");
    Serial.print(m_IpAddress[0]);
    Serial.print(".");
    Serial.print(m_IpAddress[1]);
    Serial.print(".");
    Serial.print(m_IpAddress[2]);
    Serial.print(".");
    Serial.println(m_IpAddress[3]);
}

/***********************************************************************************************************************
 */
void WmcCli::ShowNetworkSettings(void)
{
    memset(m_IpAddress, 0, 4);
    memset(m_SsidName, '\0', sizeof(m_SsidName));
    memset(m_SsidPassword, '\0', sizeof(m_SsidPassword));
    EEPROM.get(EepCfg::SsidAddress, m_SsidName);
    Serial.print(Ssid);
    Serial.println(m_SsidName);

    EEPROM.get(EepCfg::SsidPasswordAddress, m_SsidPassword);
    Serial.print(Password);
    Serial.println(m_SsidPassword);

    EEPROM.get(EepCfg::EepIpAddress, m_IpAddress);
    Serial.print(IpAdrress);
    Serial.print(m_IpAddress[0]);
    Serial.print(".");
    Serial.print(m_IpAddress[1]);
    Serial.print(".");
    Serial.print(m_IpAddress[2]);
    Serial.print(".");
    Serial.println(m_IpAddress[3]);
}

/***********************************************************************************************************************
 */
void WmcCli::Add(void)
{
    uint8_t Functions[5] = { 0, 1, 2, 3, 4 };

    m_Address = atoi(&m_bufferRx[strlen(LocAdd)]);

    if ((m_Address > 0) && (m_Address <= 9999))
    {
        /* Add loc, default functions 0..4 */
        if (m_locLib.StoreLoc(m_Address, Functions, LocLib::storeAdd) == true)
        {
            m_locLib.LocBubbleSort();
            Serial.print("Loc with address ");
            Serial.print(m_Address);
            Serial.println(" added.");
            m_wmcTft.UpdateSelectedAndNumberOfLocs(m_locLib.GetActualSelectedLocIndex(), m_locLib.GetNumberOfLocs());
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
}

/***********************************************************************************************************************
 */
void WmcCli::Delete(void)
{
    m_Address = atoi(&m_bufferRx[strlen(LocDelete)]);

    if (m_locLib.RemoveLoc(m_Address) == true)
    {
        Serial.print("Loc ");
        Serial.print(m_Address);
        Serial.println(" deleted.");
        m_wmcTft.UpdateSelectedAndNumberOfLocs(m_locLib.GetActualSelectedLocIndex(), m_locLib.GetNumberOfLocs());
    }
    else
    {
        Serial.println("Loc delete failed!");
    }
}

/***********************************************************************************************************************
 */
void WmcCli::Change(void)
{
    char* Space;
    uint8_t FunctionAssignment[5];

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
                        m_locLib.StoreLoc(m_Address, FunctionAssignment, LocLib::storeChange);
                        Serial.println("Loc function updated.");
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
}

/***********************************************************************************************************************
 */
void WmcCli::ListAllLocs(void)
{
    uint8_t Index = 0;
    LocLib::LocLibData* Data;
    char output[30];

    Serial.println("          Functions               Functions               Functions    ");
    Serial.println("Address B0 B1 B2 B3 B4  Address B0 B1 B2 B3 B4  Address B0 B1 B2 B3 B4 ");

    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);
        sprintf(output, "%4hu    %2hu %2hu %2hu %2hu %2hu  ", Data->Addres, Data->FunctionAssignment[0],
            Data->FunctionAssignment[1], Data->FunctionAssignment[2], Data->FunctionAssignment[3],
            Data->FunctionAssignment[4]);
        Serial.print(output);

        Index++;
        if ((Index % 3) == 0)
        {
            Serial.println();
        }
    }

    Serial.println();
}

/***********************************************************************************************************************
 */
void WmcCli::AcControlType(void)
{
    uint8_t AcOption;
    char* Space;

    Space = strchr(m_bufferRx, 32);

    if (Space != NULL)
    {
        Space++;

        AcOption = atoi(Space);

        switch (AcOption)
        {
        case 0:
            AcOption = 0;
            EEPROM.write(EepCfg::EepCfg::AcTypeControlAddress, AcOption);
            EEPROM.commit();
            Serial.println("AC option disabled.");
            break;
        case 1:
            AcOption = 1;
            EEPROM.write(EepCfg::EepCfg::AcTypeControlAddress, AcOption);
            EEPROM.commit();
            Serial.println("AC option enabled.");
            break;
        default: Serial.println("AC option entry invalid."); break;
        }
    }
    else
    {
        Serial.println("AC option entry invalid, must be ac 0 or ac 1");
    }
}

/***********************************************************************************************************************
 */
void WmcCli::DumpData(void)
{
    uint8_t Index = 0;
    uint8_t FunctionIndex;
    LocLib::LocLibData* Data;

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

    ShowNetworkSettings();

    /* Dump the AC option. */
    Serial.print(Ac);
    Serial.print(" ");
    Serial.println(EEPROM.read(EepCfg::EepCfg::AcTypeControlAddress));
}

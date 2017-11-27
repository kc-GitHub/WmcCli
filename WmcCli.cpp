/**
 * Functions to show data on the display of the Wmc application.
 */

// include this library's description file
#include "WmcCli.h"

const char* WmcCli::LocAdd    = "ADD";
const char* WmcCli::LocDelete = "DEL";
const char* WmcCli::LocChange = "CHANGE";
const char* WmcCli::Ssid      = "SSID";
const char* WmcCli::Password  = "PASSWORD";
const char* WmcCli::Help      = "HELP";
const char* WmcCli::LocList   = "LIST";
const char* WmcCli::Dump      = "DUMP";

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
 * Read data from serial port, if CR is received check the received data and perform action if valif command is
 * received.
 */
void WmcCli::Update(void)
{
    int DataRx;

    DataRx = Serial.read();

    while (DataRx != -1)
    {
        DataRx = toupper(DataRx);
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
    }
    else if (strncmp(m_bufferRx, Password, strlen(Password)) == 0)
    {
    }
    else if (strncmp(m_bufferRx, LocList, strlen(LocList)) == 0)
    {
        ListAllLocs();
    }
    else if (strncmp(m_bufferRx, Dump, strlen(Dump)) == 0)
    {
        DumpData();
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
    Serial.println("ADD x        : Add loc with address x");
    Serial.println("DEL x        : Delete loc with address x");
    Serial.println("CHANGE x y z : Assign function z to button y of loc with address x.");
    Serial.println("LIST         : Show all programmed locs.");
    Serial.println("DUMP         : Dump data for backup.");
    Serial.println("SSID <>      : Set SSID name (Wifi) to connect to.");
    Serial.println("PASSWORD <>  : Set password (Wifi).");
    Serial.println("HELP         : This screen.");
}

/***********************************************************************************************************************
 */
void WmcCli::Add(void)
{
    uint8_t Functions[5] = { 0, 1, 2, 3, 4 };

    m_Address = atoi(&m_bufferRx[strlen(LocAdd)]);

    if ((m_Address > 0) && (m_Address < 9999))
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
    m_Address = atoi(&m_bufferRx[strlen(LocAdd)]);

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
void WmcCli::DumpData(void)
{
    uint8_t Index = 0;
    uint8_t FunctionIndex;
    LocLib::LocLibData* Data;

    while (Index < m_locLib.GetNumberOfLocs())
    {
        Data = m_locLib.LocGetAllDataByIndex(Index);
        Serial.print("ADD ");
        Serial.println(Data->Addres);

        for (FunctionIndex = 0; FunctionIndex < 5; FunctionIndex++)
        {
            Serial.print("CHANGE ");
            Serial.print(Data->Addres);
            Serial.print(" ");
            Serial.print(FunctionIndex);
            Serial.print(" ");
            Serial.println(Data->FunctionAssignment[FunctionIndex]);
        }

        Index++;
    }
}

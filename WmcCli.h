/**
 **********************************************************************************************************************
 * @file  WmcCli.h
 * @brief Command line interfacve for the Wmc application.
 ***********************************************************************************************************************
 */

#ifndef WMC_CLI_H
#define WMC_CLI_H

#include "Loclib.h"
#include "WmcCli.h"
#include "WmcTft.h"
#include <Arduino.h>

class WmcCli
{
public:
    /* Constructor */
    WmcCli();

    /**
     * Init the cli module.
     */
    void Init(void);

    /**
     * Update the cli module.
     */
    void Update(void);

private:
    /**
     * Check an process received command.
     */
    void Process(void);

    /**
     * Show help screen.
     */
    void HelpScreen(void);

    /**
     * Write SSID name.
     */
    void SsIdWriteName(void);

    /**
     * Write SSID password.
     */
    void SsIdWritePassword(void);

    /**
     * Try to add loc.
     */
    void Add(void);

    /**
     * Try to delete loc.
     */
    void Delete(void);

    /**
     * Change loc data (function assignment).
     */
    void Change(void);

    /**
     * List programmed locs.
     */
    void ListAllLocs(void);

    /**
     * Dump data for backup.
     */
    void DumpData(void);

    LocLib m_locLib;

    WmcTft m_wmcTft;

    /**
     * Receive buffer.
     */
    char m_bufferRx[40];
    uint16_t m_bufferRxIndex;
    uint16_t m_Address;
    uint16_t m_DecoderSteps;
    uint16_t m_Function;
    uint16_t m_Button;
    char m_NameStr[10];
    char m_SsidName[50];
    char m_SsidPassword[50];

    static const char* LocAdd;
    static const char* LocDelete;
    static const char* LocChange;
    static const char* Ssid;
    static const char* Password;
    static const char* Help;
    static const char* LocList;
    static const char* Dump;
};

#endif

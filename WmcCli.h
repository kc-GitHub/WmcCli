/**
 **********************************************************************************************************************
 * @file  WmcCli.h
 * @brief Command line interfacve for the Wmc application.
 ***********************************************************************************************************************
 */

#ifndef WMC_CLI_H
#define WMC_CLI_H

/***********************************************************************************************************************
 * I N C L U D E S
 **********************************************************************************************************************/
#include "Loclib.h"
#include "app_cfg.h"

#if APP_CFG_UC == APP_CFG_UC_ESP8266
#include "wmc_event.h"
#include <ESPTelnet.h>

#else
#include "xmc_event.h"
#endif
#include <Arduino.h>

// Macro for suppress unused parameter compiler warning in c++.
#define UNUSED(x) (void)(x)

/***********************************************************************************************************************
 * T Y P E D E F S  /  E N U M
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * C L A S S E S
 **********************************************************************************************************************/
class WmcCli
{
public:
    /* Constructor */
    WmcCli();

    /**
     * Init the cli module.
     */
    void Init(LocLib LocLib, LocStorage LocStorage);

    size_t print(const char[]);
    size_t print(const String &);
    size_t print(int, int = DEC);
    size_t print(unsigned char, int = DEC);

    size_t println(unsigned char, int = DEC);
    size_t println(int, int = DEC);
    size_t println(const String &s);

    size_t println(void);

    /**
     * Update the cli module.
     */
    void Update(void);

    /**
     * Check an process received command.
     */
    void Process(char *bufferRx);

private:
    /**
     * Check an process received command.
     */
    void Process(void);

    /**
     * Show help screen.
     */
    void HelpScreen(void);
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    /**
     * Write IP address to connect to.
     */
    bool IpAddressWriteZ21(void);

    /**
     * Show programmed IP settings.
     */
    void ShowNetworkSettings(void);
#endif
    /**
     * Try to add loc.
     */
    bool Add(void);

    /**
     * Try to delete loc.
     */
    bool Delete(void);

    /**
     * Change loc data (function assignment).
     */
    bool Change(void);

    /**
     * List programmed locs.
     */
    void ListAllLocs(void);

    /**
     * Set name of loc.
     */
    bool SetName(void);

    /**
     * Set control type, normal or AC.
     */
    bool AcControlType(void);

    /**
     * Set control emergency option
     */
    bool EmergencyChange(void);

    /**
     * Dump data for backup.
     */
    void DumpData(void);

    /**
     * Show overview of settings.
     */
    void ShowSettings(void);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
     /**
      * Retrieve Ip data from string.
      */
     bool IpGetData(const char* SourcePtr, uint8_t* TargetPtr);

#endif

    /**
     * Print ip data.
     */
    void IpDataPrint(const char* StrPtr, uint8_t* IpDataPtr);

    LocLib m_locLib;
    LocStorage m_LocStorage;
    char m_bufferRx[75];
    uint16_t m_bufferRxIndex;
    uint16_t m_Address;
    uint16_t m_DecoderSteps;
    uint16_t m_Function;
    uint16_t m_Button;
    char m_NameStr[10];
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    uint8_t m_IpAddressZ21[4];
#endif

    static const char* LocAdd;
    static const char* LocDelete;
    static const char* LocChange;
    static const char* LocName;
    static const char* LocDeleteAll;
    static const char* EraseAll;
    static const char* Emergency;
    static const char* Help;
    static const char* LocList;
    static const char* Ac;
    static const char* Dump;
    static const char* Settings;
    static const char* Reset;
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    static const char* Exit;
    static const char* Ssid;
    static const char* IpAdrressZ21;
    static const char* Network;
#endif

    cliEnterEvent Event;
};

#endif

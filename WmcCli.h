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
#else
#include "xmc_event.h"
#endif
#include <Arduino.h>

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

    /**
     * Update the cli module.
     */
    void Update(void);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    /**
     * Default IP settings.
     */
    void IpSettingsDefault(void);
#endif

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
     * Write SSID name.
     */
    void SsIdWriteName(void);

    /**
     * Write SSID password.
     */
    void SsIdWritePassword(void);

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
     * Set control type, normal or AC.
     */
    bool AcControlType(void);

#if APP_CFG_UC == APP_CFG_UC_ESP8266
    /**
     * Toggle between static and dynamic ip address.
     */
    bool StaticIpChange(void);

    /**
     * Write fixed (static) IP address of WMC.
     */
    bool IpAddressWriteWmc(void);

    /**
     * Write IP gateway of WMC.
     */
    bool IpAddressWriteGateway(void);

    /**
     * Write IP subnet of WMC.
     */
    bool IpAddressWriteSubnet(void);
#endif
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

    /**
     * Print ip data.
     */
    void IpDataPrint(const char* StrPtr, uint8_t* IpDataPtr);

#endif
    LocLib m_locLib;
    LocStorage m_LocStorage;
    char m_bufferRx[40];
    uint16_t m_bufferRxIndex;
    uint16_t m_Address;
    uint16_t m_DecoderSteps;
    uint16_t m_Function;
    uint16_t m_Button;
    char m_NameStr[10];
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    char m_SsidName[40];
    char m_SsidPassword[40];
    uint8_t m_IpAddressZ21[4];
    uint8_t m_IpAddresWmc[4];
    uint8_t m_IpGateway[4];
    uint8_t m_IpSubnet[4];
#endif

    static const char* LocAdd;
    static const char* LocDelete;
    static const char* LocChange;
    static const char* LocDeleteAll;
    static const char* EraseAll;
    static const char* Help;
    static const char* LocList;
    static const char* Ac;
    static const char* Dump;
    static const char* Settings;
#if APP_CFG_UC == APP_CFG_UC_ESP8266
    static const char* Ssid;
    static const char* Password;
    static const char* IpAdrressZ21;
    static const char* Network;
    static const char* Ip;
    static const char* Gateway;
    static const char* Subnet;
    static const char* StaticIp;
#endif

    cliEnterEvent Event;
};

#endif

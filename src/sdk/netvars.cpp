#include "../includes.hpp"
#include "interfaces/ibaseclientdll.hpp"
#include "interfaces/interfaces.hpp"
#include "netvars.hpp"
#include <cstdint>
#include <cstring>

uintptr_t getOffset(RecvTable* table, const char* tableName, const char* netvarName) {
    for (int i = 0; i < table->m_nProps; i++) {
        RecvProp prop = table->m_pProps[i];

        if (!strcmp(prop.m_pVarName, netvarName)) {
            return prop.m_Offset;
        }

        if (prop.m_pDataTable) {
            uintptr_t offset = getOffset(prop.m_pDataTable, tableName, netvarName);

            if (offset) {
                return offset + prop.m_Offset;
            }
        }
    }
    return 0;
}

uintptr_t getNetvarOffset(const char* tableName, const char* netvarName) {
    for (ClientClass* cur = Interfaces::client->GetAllClasses(); cur; cur = cur->m_pNext) {
        if (!strcmp(tableName, cur->m_pRecvTable->m_pNetTableName)) {
            return getOffset(cur->m_pRecvTable, tableName, netvarName);
        }
    }
    return 0;
}

bool Netvar::init() {
    Log::log(LOG, "Initialising Netvars...");
    for (auto& nvar: offsets) {
        nvar.second = getNetvarOffset(nvar.first.first.data(), nvar.first.second.data());
        Log::log(LOG, " %s %s | %lx", nvar.first.first.data(), nvar.first.second.data(), nvar.second);
    }

    Log::log(LOG, "Testing netvars by getting m_iHealth from DT_BasePlayer (%lx)", GETNETVAROFFSET("DT_BasePlayer", "m_iHealth"));
    if (GETNETVAROFFSET("DT_BasePlayer", "m_iHealth") != 0x138) {
        Log::log(WARN, "When getting m_iHealth from DT_BasePlayer the value isnt as expected, this could mean there is a problem in getting netvars or the game has just updated.");
    }

    Log::log(LOG, "Initialising offsets");

    Offsets::sendClantag = (Offsets::SendClantag)PatternScan::findFirstInModule("engine_client.so",
            "55 48 89 E5 41 55 49 89 FD 41 54 BF");
    Log::log(LOG, " sendClantag | %lx", Offsets::sendClantag);

    Offsets::setPlayerReady = (Offsets::SetPlayerReady)PatternScan::findFirstInModule("/client_client.so",
            "55 48 89 F7 48 8D 35 ? ? ? ? 48 89 E5 E8 ? ? ? ? 85 C0");
    Log::log(LOG, " setPlayerReady | %lx", Offsets::setPlayerReady);

    Offsets::radarIsHltvCheck = PatternScan::findFirstInModule("/client_client.so", "84 C0 74 50 31 F6");
    Log::log(LOG, " radarIsHltvCheck | %lx", Offsets::radarIsHltvCheck);

    Offsets::initKeyValues = (Offsets::InitKeyValues)PatternScan::findFirstInModule("/client_client.so", 
            "81 27 00 00 00 FF 55 31 C0 48 89 E5 5D");
    Log::log(LOG, " initKeyValues | %lx", Offsets::initKeyValues);

    Offsets::loadFromBuffer = (Offsets::LoadFromBuffer)PatternScan::findFirstInModule("/client_client.so", 
            "55 48 89 E5 41 57 41 56 41 55 41 54 49 89 D4 53 48 81 EC ? ? ? ? 48 85"),
    Log::log(LOG, " loadFromBuffer | %lx", Offsets::loadFromBuffer);

    Offsets::setNamedSkybox = (Offsets::SetNamedSkybox)PatternScan::findFirstInModule("engine_client.so", 
            "55 4C 8D 05 ? ? ? ? 48 89 E5 41");
    Log::log(LOG, " setNamedSkybox | %lx", Offsets::setNamedSkybox);

    Offsets::lineGoesThroughSmoke = (Offsets::LineGoesThroughSmoke)PatternScan::findFirstInModule("/client_client.so", 
            "40 0F B6 FF 55");
    Log::log(LOG, " lineGoesThroughSmoke | %lx", Offsets::lineGoesThroughSmoke);

    return true;
}
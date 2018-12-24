/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include "idm.h"

const char* CGameControllerIDM::GAMETYPE_NAME = "idm";

CGameControllerIDM::CGameControllerIDM(class CGameContext *pGameServer)
    : CGameControllerDM(pGameServer)
{
    // Exchange this to a string that identifies your game mode.
    // DM, TDM and CTF are reserved for teeworlds original modes.
    m_pGameType = CGameControllerIDM::GAMETYPE_NAME;
    m_ForcedWeapon = WEAPON_LASER;

    //m_GameFlags = GAMEFLAG_TEAMS; // GAMEFLAG_TEAMS makes it a two-team gamemode
}

void CGameControllerIDM::Tick()
{
    CGameControllerDM::Tick();
}

int CGameControllerIDM::OnCharacterDeath(CCharacter * pVictim, CPlayer * pKiller, int Weapon)
{
    // Kill streak
    pKiller->AddToKillStreak();

    if (pVictim->GetPlayer()->GetKillStreak() > 4) {
        IServer::CClientInfo victimInfo;
        Server()->GetClientInfo(pVictim->GetPlayer()->GetCID(), &victimInfo);

        char aBuf[128];
        str_format(
            aBuf, sizeof(aBuf),
            "'%s' has been killed with %i killstreak on account!",
            victimInfo.m_pName, pVictim->GetPlayer()->GetKillStreak()
        );
        GameServer()->SendChat(-1, CHAT_ALL, -1, aBuf);
        GameServer()->CreateSound(pVictim->GetPos(), SOUND_GRENADE_EXPLODE);
    }

    return CGameControllerDM::OnCharacterDeath(pVictim, pKiller, Weapon);
}

void CGameControllerIDM::OnCharacterSpawn(CCharacter * pChr)
{
    CGameControllerDM::OnCharacterSpawn(pChr);

    pChr->GetPlayer()->ResetKillStreak();

    // Take weapons
    for (int i = 0; i < NUM_WEAPONS; i++)
        pChr->TakeWeapon(i);

    // Give and Set weapon to Laser
    pChr->GiveWeapon(m_ForcedWeapon, -1);
    pChr->SetWeapon(m_ForcedWeapon);
}

void CGameControllerIDM::OnMatchEnd()
{
    PrintHighestKillStreak();
}

void CGameControllerIDM::PrintHighestKillStreak()
{
    int BiggestKillstreak = 0;
    int BiggestKillstreakClientID = -1;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (GameServer()->m_apPlayers[i]) {
            CPlayer *player = GameServer()->m_apPlayers[i];
            int playerKillstreak = player->GetMaxKillStreak();

            if (playerKillstreak > BiggestKillstreak) {
                BiggestKillstreak = playerKillstreak;
                BiggestKillstreakClientID = player->GetCID();
            }
        }
    }

    if (BiggestKillstreakClientID >= 0) {
        IServer::CClientInfo clientInfo;
        Server()->GetClientInfo(BiggestKillstreakClientID, &clientInfo);

        char aBuf[128];
        str_format(
            aBuf, sizeof(aBuf),
            "'%s' had biggest killstreak in match (%i killstreak)",
            clientInfo.m_pName, BiggestKillstreak
        );
        GameServer()->SendChat(-1, CHAT_ALL, -1, aBuf);
    }
}
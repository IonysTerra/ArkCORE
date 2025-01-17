/*
 * Copyright (C) 2005 - 2011 MaNGOS <http://www.getmangos.org/>
 *
 * Copyright (C) 2008 - 2011 TrinityCore <http://www.trinitycore.org/>
 *
 * Copyright (C) 2011 ArkCORE <http://www.arkania.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Battleground.h"
#include "BattlegroundDS.h"
#include "Language.h"
#include "Player.h"
#include "Object.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"

BattlegroundDS::BattlegroundDS()
{
    m_BgObjects.resize(BG_DS_OBJECT_MAX);
    m_StartDelayTimes[BG_STARTING_EVENT_FIRST]     = BG_START_DELAY_1M;
    m_StartDelayTimes[BG_STARTING_EVENT_SECOND]    = BG_START_DELAY_30S;
    m_StartDelayTimes[BG_STARTING_EVENT_THIRD]     = BG_START_DELAY_15S;
    m_StartDelayTimes[BG_STARTING_EVENT_FOURTH]    = BG_START_DELAY_NONE;
    //we must set messageIds
    m_StartMessageIds[BG_STARTING_EVENT_FIRST]     = LANG_ARENA_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND]    = LANG_ARENA_THIRTY_SECONDS;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD]     = LANG_ARENA_FIFTEEN_SECONDS;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH]    = LANG_ARENA_HAS_BEGUN;
    m_knockback = 5000;
    m_knockbackCheck = true;
}

BattlegroundDS::~BattlegroundDS() {}

void BattlegroundDS::PostUpdateImpl(uint32 diff)
{
    if (GetStartTime() >= 75*IN_MILLISECONDS)
       {
            for(BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end();itr++)
            {
                Player *plr = ObjectAccessor::FindPlayer(itr->first);
                if (plr && plr->isAlive() && plr->GetPositionX() < 1260 && plr->GetPositionY() >755 && plr->GetPositionY() < 775 && plr->GetPositionZ() > 13)
                {
                    KnockBackPlayer(plr, 6.15f, 50.00f, 5.00f);
                    plr->RemoveAurasDueToSpell(48018);
                }
                if (plr && plr->isAlive() && plr->GetPositionX() > 1330 && plr->GetPositionY() >805 && plr->GetPositionY() < 825 && plr->GetPositionZ() > 13)
                {
                    KnockBackPlayer(plr, 3.10f, 50.00f, 5.00f);
                    plr->RemoveAurasDueToSpell(48018);
                }
            }
        }
        if (getWaterFallTimer() < diff)
        {
            if (isWaterFallActive())
            {
                setWaterFallTimer(urand(BG_DS_WATERFALL_TIMER_MIN, BG_DS_WATERFALL_TIMER_MAX));
                for (uint32 i = BG_DS_OBJECT_WATER_1; i <= BG_DS_OBJECT_WATER_2; ++i)
                    SpawnBGObject(i, getWaterFallTimer());
                setWaterFallActive(false);
            }
            else
            {
                setWaterFallTimer(BG_DS_WATERFALL_DURATION);
                for (uint32 i = BG_DS_OBJECT_WATER_1; i <= BG_DS_OBJECT_WATER_2; ++i)
                    SpawnBGObject(i, RESPAWN_IMMEDIATELY);
                setWaterFallActive(true);
            }
        }
        else
            setWaterFallTimer(getWaterFallTimer() - diff);
}

void BattlegroundDS::StartingEventCloseDoors()
{
    for (uint32 i = BG_DS_OBJECT_DOOR_1; i <= BG_DS_OBJECT_DOOR_2; ++i)
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
}

void BattlegroundDS::StartingEventOpenDoors()
{
    for (uint32 i = BG_DS_OBJECT_DOOR_1; i <= BG_DS_OBJECT_DOOR_2; ++i)
        DoorOpen(i);

    for (uint32 i = BG_DS_OBJECT_BUFF_1; i <= BG_DS_OBJECT_BUFF_2; ++i)
        SpawnBGObject(i, 60);

    setWaterFallTimer(urand(BG_DS_WATERFALL_TIMER_MIN, BG_DS_WATERFALL_TIMER_MAX));
    setWaterFallActive(false);

    for (uint32 i = BG_DS_OBJECT_WATER_1; i <= BG_DS_OBJECT_WATER_2; ++i)
        SpawnBGObject(i, getWaterFallTimer());

	m_knockback = 5000;
    m_knockbackCheck = true;		
}

void BattlegroundDS::AddPlayer(Player *plr)
{
    Battleground::AddPlayer(plr);
    //create score and add it to map, default values are set in constructor
    BattlegroundDSScore* sc = new BattlegroundDSScore;

    m_PlayerScores[plr->GetGUID()] = sc;

    UpdateArenaWorldState();
}

void BattlegroundDS::RemovePlayer(Player* /*plr*/, uint64 /*guid*/, uint32 /*team*/)
{
    if (GetStatus() == STATUS_WAIT_LEAVE)
        return;

    UpdateArenaWorldState();
    CheckArenaWinConditions();
}

void BattlegroundDS::HandleKillPlayer(Player* player, Player* killer)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    if (!killer)
    {
        sLog->outError("BattlegroundDS: Killer player not found");
        return;
    }

    Battleground::HandleKillPlayer(player, killer);

    UpdateArenaWorldState();
    CheckArenaWinConditions();
}

void BattlegroundDS::HandleAreaTrigger(Player *Source, uint32 Trigger)
{
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;

    switch (Trigger)
    {
        case 5347:
        case 5348:
            break;
        default:
            sLog->outError("WARNING: Unhandled AreaTrigger in Battleground: %u", Trigger);
            Source->GetSession()->SendAreaTriggerMessage("Warning: Unhandled AreaTrigger in Battleground: %u", Trigger);
            break;
    }
}

bool BattlegroundDS::HandlePlayerUnderMap(Player* player)
{
    player->TeleportTo(GetMapId(), 1299.046f, 784.825f, 9.338f, 2.422f, false);
    return true;
}

void BattlegroundDS::FillInitialWorldStates(WorldPacket &data)
{
    data << uint32(3610) << uint32(1); // 9 show
    UpdateArenaWorldState();
}

void BattlegroundDS::Reset()
{
    //call parent's class reset
    Battleground::Reset();
    m_knockback = 5000;
    m_knockbackCheck = true;
}

bool BattlegroundDS::SetupBattleground()
{
    // gates
    if (!AddObject(BG_DS_OBJECT_DOOR_1, BG_DS_OBJECT_TYPE_DOOR_1, 1350.95f, 817.2f, 20.8096f, 3.15f, 0, 0, 0.99627f, 0.0862864f, RESPAWN_IMMEDIATELY)
        || !AddObject(BG_DS_OBJECT_DOOR_2, BG_DS_OBJECT_TYPE_DOOR_2, 1232.65f, 764.913f, 20.0729f, 6.3f, 0, 0, 0.0310211f, -0.999519f, RESPAWN_IMMEDIATELY)
    // water
        || !AddObject(BG_DS_OBJECT_WATER_1, BG_DS_OBJECT_TYPE_WATER_1, 1291.56f, 790.837f, 7.1f, 3.14238f, 0, 0, 0.694215f, -0.719768f, 120)
        || !AddObject(BG_DS_OBJECT_WATER_2, BG_DS_OBJECT_TYPE_WATER_2, 1291.56f, 790.837f, 7.1f, 3.14238f, 0, 0, 0.694215f, -0.719768f, 120)
    // buffs
        || !AddObject(BG_DS_OBJECT_BUFF_1, BG_DS_OBJECT_TYPE_BUFF_1, 1291.7f, 813.424f, 7.11472f, 4.64562f, 0, 0, 0.730314f, -0.683111f, 120)
        || !AddObject(BG_DS_OBJECT_BUFF_2, BG_DS_OBJECT_TYPE_BUFF_2, 1291.7f, 768.911f, 7.11472f, 1.55194f, 0, 0, 0.700409f, 0.713742f, 120))
    {
        sLog->outErrorDb("BatteGroundDS: Failed to spawn some object!");
        return false;
    }
    return true;
}

void BattlegroundDS::KnockBackPlayer(Unit *pPlayer, float angle, float horizontalSpeed, float verticalSpeed)
{
    if(pPlayer->GetTypeId() == TYPEID_PLAYER)
    {
        WorldPacket data(SMSG_MOVE_KNOCK_BACK, 8+4+4+4+4+2);   // this needs checked for cataclysm!
        data.append(pPlayer->GetPackGUID());
        data << uint32(0);
        data << float(cos(angle));
        data << float(sin(angle));
        data << float(horizontalSpeed);
        data << float(-verticalSpeed);
        ((Player*)pPlayer)->GetSession()->SendPacket(&data);
    }
    else
        sLog->outError("The target of KnockBackPlayer must be a player !");	
}

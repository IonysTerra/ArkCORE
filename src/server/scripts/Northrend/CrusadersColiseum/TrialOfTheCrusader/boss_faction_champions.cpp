/*
 * Copyright (C) 2011 ArkCORE <http://www.arkania.net/>

 * Copyright (C) 2006-2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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

#include "ScriptPCH.h"
#include "trial_of_the_crusader.h"

enum eYell
{
    SAY_GARROSH_KILL_ALLIANCE_PLAYER4 = -1649118,
    SAY_VARIAN_KILL_HORDE_PLAYER4     = -1649123,
};

enum eAIs
{
    AI_MELEE    = 0,
    AI_RANGED   = 1,
    AI_HEALER   = 2,
    AI_PET      = 3,
};

enum eSpells
{
    SPELL_ANTI_AOE      = 68595,
    SPELL_PVP_TRINKET   = 65547,
};

class boss_toc_champion_controller : public CreatureScript
{
public:
    boss_toc_champion_controller() : CreatureScript("boss_toc_champion_controller") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_toc_champion_controllerAI (creature);
    }

    struct boss_toc_champion_controllerAI : public ScriptedAI
    {
        boss_toc_champion_controllerAI(Creature* creature) : ScriptedAI(creature), Summons(me)
        {
            m_pInstance = (InstanceScript* ) creature->GetInstanceScript();
        }

        InstanceScript* m_pInstance;
        SummonList Summons;
        uint32 m_uiChampionsNotStarted;
        uint32 m_uiChampionsFailed;
        uint32 m_uiChampionsKilled;
        bool   m_bInProgress;

        void Reset()
        {
            m_uiChampionsNotStarted = 0;
            m_uiChampionsFailed = 0;
            m_uiChampionsKilled = 0;
            m_bInProgress = false;
        }

        std::vector<uint32> SelectChampions(Team playerTeam)
        {
            std::vector<uint32> vHealersEntries;
            vHealersEntries.clear();
            vHealersEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_DRUID_RESTORATION : NPC_ALLIANCE_DRUID_RESTORATION);
            vHealersEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_PALADIN_HOLY : NPC_ALLIANCE_PALADIN_HOLY);
            vHealersEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_PRIEST_DISCIPLINE : NPC_ALLIANCE_PRIEST_DISCIPLINE);
            vHealersEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_SHAMAN_RESTORATION : NPC_ALLIANCE_SHAMAN_RESTORATION);

            std::vector<uint32> vOtherEntries;
            vOtherEntries.clear();
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_DEATH_KNIGHT : NPC_ALLIANCE_DEATH_KNIGHT);
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_HUNTER : NPC_ALLIANCE_HUNTER);
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_MAGE : NPC_ALLIANCE_MAGE);
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_ROGUE : NPC_ALLIANCE_ROGUE);
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_WARLOCK : NPC_ALLIANCE_WARLOCK);
            vOtherEntries.push_back(playerTeam == ALLIANCE ? NPC_HORDE_WARRIOR : NPC_ALLIANCE_WARRIOR);

            uint8 healersSubtracted = 2;
            if (m_pInstance->instance->GetSpawnMode() == RAID_DIFFICULTY_25MAN_NORMAL || m_pInstance->instance->GetSpawnMode() == RAID_DIFFICULTY_25MAN_HEROIC)
                healersSubtracted = 1;
            for (uint8 i = 0; i < healersSubtracted; ++i)
            {
                uint8 pos = urand(0, vHealersEntries.size()-1);
                switch (vHealersEntries[pos])
                {
                    case NPC_ALLIANCE_DRUID_RESTORATION:
                        vOtherEntries.push_back(NPC_ALLIANCE_DRUID_BALANCE);
                        break;
                    case NPC_HORDE_DRUID_RESTORATION:
                        vOtherEntries.push_back(NPC_HORDE_DRUID_BALANCE);
                        break;
                    case NPC_ALLIANCE_PALADIN_HOLY:
                        vOtherEntries.push_back(NPC_ALLIANCE_PALADIN_RETRIBUTION);
                        break;
                    case NPC_HORDE_PALADIN_HOLY:
                        vOtherEntries.push_back(NPC_HORDE_PALADIN_RETRIBUTION);
                        break;
                    case NPC_ALLIANCE_PRIEST_DISCIPLINE:
                        vOtherEntries.push_back(NPC_ALLIANCE_PRIEST_SHADOW);
                        break;
                    case NPC_HORDE_PRIEST_DISCIPLINE:
                        vOtherEntries.push_back(NPC_HORDE_PRIEST_SHADOW);
                        break;
                    case NPC_ALLIANCE_SHAMAN_RESTORATION:
                        vOtherEntries.push_back(NPC_ALLIANCE_SHAMAN_ENHANCEMENT);
                        break;
                    case NPC_HORDE_SHAMAN_RESTORATION:
                        vOtherEntries.push_back(NPC_HORDE_SHAMAN_ENHANCEMENT);
                        break;
                }
                vHealersEntries.erase(vHealersEntries.begin()+pos);
            }

            if (m_pInstance->instance->GetSpawnMode() == RAID_DIFFICULTY_10MAN_NORMAL || m_pInstance->instance->GetSpawnMode() == RAID_DIFFICULTY_10MAN_HEROIC)
                for (uint8 i = 0; i < 4; ++i)
                    vOtherEntries.erase(vOtherEntries.begin()+urand(0, vOtherEntries.size()-1));

            std::vector<uint32> vChampionEntries;
            vChampionEntries.clear();
            for (uint8 i = 0; i < vHealersEntries.size(); ++i)
                vChampionEntries.push_back(vHealersEntries[i]);
            for (uint8 i = 0; i < vOtherEntries.size(); ++i)
                vChampionEntries.push_back(vOtherEntries[i]);

            return vChampionEntries;
        }

        void SummonChampions(Team playerTeam)
        {
            std::vector<Position> vChampionJumpOrigin;
            if (playerTeam == ALLIANCE)
                for (uint8 i = 0; i < 5; i++)
                    vChampionJumpOrigin.push_back(FactionChampionLoc[i]);
            else
                for (uint8 i = 5; i < 10; i++)
                    vChampionJumpOrigin.push_back(FactionChampionLoc[i]);

            std::vector<Position> vChampionJumpTarget;
            for (uint8 i = 10; i < 20; i++)
                vChampionJumpTarget.push_back(FactionChampionLoc[i]);
            std::vector<uint32> vChampionEntries = SelectChampions(playerTeam);

            for (uint8 i = 0; i < vChampionEntries.size(); ++i)
            {
                uint8 pos = urand(0, vChampionJumpTarget.size()-1);
                if (Creature* pTemp = me->SummonCreature(vChampionEntries[i], vChampionJumpOrigin[urand(0, vChampionJumpOrigin.size()-1)], TEMPSUMMON_MANUAL_DESPAWN))
                {
                    Summons.Summon(pTemp);
                    pTemp->SetReactState(REACT_PASSIVE);
                    pTemp->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE);
                    if (playerTeam == ALLIANCE)
                    {
                        pTemp->SetHomePosition(vChampionJumpTarget[pos].GetPositionX(), vChampionJumpTarget[pos].GetPositionY(), vChampionJumpTarget[pos].GetPositionZ(), 0);
                        pTemp->GetMotionMaster()->MoveJump(vChampionJumpTarget[pos].GetPositionX(), vChampionJumpTarget[pos].GetPositionY(), vChampionJumpTarget[pos].GetPositionZ(), 20.0f, 20.0f);
                        pTemp->SetOrientation(0);
                    }
                    else
                    {
                        pTemp->SetHomePosition((ToCCommonLoc[1].GetPositionX()*2)-vChampionJumpTarget[pos].GetPositionX(), vChampionJumpTarget[pos].GetPositionY(), vChampionJumpTarget[pos].GetPositionZ(), 3);
                        pTemp->GetMotionMaster()->MoveJump((ToCCommonLoc[1].GetPositionX()*2)-vChampionJumpTarget[pos].GetPositionX(), vChampionJumpTarget[pos].GetPositionY(), vChampionJumpTarget[pos].GetPositionZ(), 20.0f, 20.0f);
                        pTemp->SetOrientation(3);
                    }
                }
                vChampionJumpTarget.erase(vChampionJumpTarget.begin()+pos);
            }
        }

        void SetData(uint32 uiType, uint32 uiData)
        {
            switch (uiType)
            {
                case 0:
                    SummonChampions((Team)uiData);
                    break;
                case 1:
                    for (std::list<uint64>::iterator i = Summons.begin(); i != Summons.end(); ++i)
                    {
                        if (Creature* pTemp = Unit::GetCreature(*me, *i))
                        {
                            pTemp->SetReactState(REACT_AGGRESSIVE);
                            pTemp->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE);
                        }
                    }
                    break;
                case 2:
                    switch (uiData)
                    {
                        case FAIL:
                            m_uiChampionsFailed++;
                            if (m_uiChampionsFailed + m_uiChampionsKilled >= Summons.size())
                            {
                                m_pInstance->SetData(TYPE_CRUSADERS, FAIL);
                                Summons.DespawnAll();
                                me->DespawnOrUnsummon();
                            }
                            break;
                        case IN_PROGRESS:
                            if (!m_bInProgress)
                            {
                                m_uiChampionsNotStarted = 0;
                                m_uiChampionsFailed = 0;
                                m_uiChampionsKilled = 0;
                                m_bInProgress = true;
                                Summons.DoZoneInCombat();
                                m_pInstance->SetData(TYPE_CRUSADERS, IN_PROGRESS);
                            }
                            break;
                        case DONE:
                            m_uiChampionsKilled++;
                            if (m_uiChampionsKilled == 1)
                                m_pInstance->SetData(TYPE_CRUSADERS, SPECIAL);
                            else if (m_uiChampionsKilled >= Summons.size())
                            {
                                m_pInstance->SetData(TYPE_CRUSADERS, DONE);
                                Summons.DespawnAll();
                                me->DespawnOrUnsummon();
                            }
                            break;
                    }
                    break;
            }
        }
    };
};

struct boss_faction_championsAI : public ScriptedAI
{
    boss_faction_championsAI(Creature* creature, uint32 aitype) : ScriptedAI(creature)
    {
        m_pInstance = (InstanceScript* ) creature->GetInstanceScript();
        mAIType = aitype;
    }

    InstanceScript* m_pInstance;

    uint64 championControllerGUID;
    uint32 mAIType;
    uint32 ThreatTimer;
    uint32 CCTimer;

    void Reset()
    {
        championControllerGUID = 0;
        CCTimer = rand()%10000;
        ThreatTimer = 5000;
    }

    void JustReachedHome()
    {
        if (m_pInstance)
            if (Creature* pChampionController = Unit::GetCreature((*me), m_pInstance->GetData64(NPC_CHAMPIONS_CONTROLLER)))
                pChampionController->AI()->SetData(2, FAIL);
        me->DespawnOrUnsummon();
    }

    float CalculateThreat(float distance, float armor, uint32 health)
    {
        float dist_mod = (mAIType == AI_MELEE || mAIType == AI_PET) ? 15.0f/(15.0f + distance) : 1.0f;
        float armor_mod = (mAIType == AI_MELEE || mAIType == AI_PET) ? armor / 16635.0f : 0.0f;
        float eh = (health+1) * (1.0f + armor_mod);
        return dist_mod * 30000.0f / eh;
    }

    void UpdateThreat()
    {
        std::list<HostileReference*> const& tList = me->getThreatManager().getThreatList();
        for (std::list<HostileReference*>::const_iterator itr = tList.begin(); itr != tList.end(); ++itr)
        {
            Unit* pUnit = Unit::GetUnit((*me), (*itr)->getUnitGuid());
            if (pUnit && me->getThreatManager().getThreat(pUnit))
            {
                if (pUnit->GetTypeId()==TYPEID_PLAYER)
                {
                    float threat = CalculateThreat(me->GetDistance2d(pUnit), (float)pUnit->GetArmor(), pUnit->GetHealth());
                    me->getThreatManager().modifyThreatPercent(pUnit, -100);
                    me->AddThreat(pUnit, 1000000.0f * threat);
                }
            }
        }
    }

    void UpdatePower()
    {
        if (me->getPowerType() == POWER_MANA)
            me->ModifyPower(POWER_MANA, me->GetMaxPower(POWER_MANA) / 3);
        //else if (me->getPowerType() == POWER_ENERGY)
        //    me->ModifyPower(POWER_ENERGY, 100);
    }

    void RemoveCC()
    {
        me->RemoveAurasByType(SPELL_AURA_MOD_STUN);
        me->RemoveAurasByType(SPELL_AURA_MOD_FEAR);
        me->RemoveAurasByType(SPELL_AURA_MOD_ROOT);
        me->RemoveAurasByType(SPELL_AURA_MOD_PACIFY);
        me->RemoveAurasByType(SPELL_AURA_MOD_CONFUSE);
        //DoCast(me, SPELL_PVP_TRINKET);
    }

    void JustDied(Unit* /*killer*/)
    {
        if (mAIType != AI_PET)
            if (m_pInstance)
                if (Creature* pChampionController = Unit::GetCreature((*me), m_pInstance->GetData64(NPC_CHAMPIONS_CONTROLLER)))
                    pChampionController->AI()->SetData(2, DONE);
    }

    void EnterCombat(Unit* /*who*/)
    {
        DoCast(me, SPELL_ANTI_AOE, true);
        me->SetInCombatWithZone();
        if (m_pInstance)
            if (Creature* pChampionController = Unit::GetCreature((*me), m_pInstance->GetData64(NPC_CHAMPIONS_CONTROLLER)))
                pChampionController->AI()->SetData(2, IN_PROGRESS);
    }

    void KilledUnit(Unit* who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            Map::PlayerList const &players = me->GetMap()->GetPlayers();
            uint32 TeamInInstance = 0;

            if (!players.isEmpty())
                if (Player* player = players.begin()->getSource())
                    TeamInInstance = player->GetTeam();

            if (m_pInstance)
            {
                if (TeamInInstance == ALLIANCE)
                {
                    if (Creature* pTemp = Unit::GetCreature(*me, m_pInstance->GetData64(NPC_VARIAN)))
                        DoScriptText(SAY_VARIAN_KILL_HORDE_PLAYER4+urand(0, 3), pTemp); // + cause we are on negative
                }
                else
                    if (Creature* pTemp = me->FindNearestCreature(NPC_GARROSH, 300.f))
                        DoScriptText(SAY_GARROSH_KILL_ALLIANCE_PLAYER4+urand(0, 3), pTemp); // + cause we are on negative

                m_pInstance->SetData(DATA_TRIBUTE_TO_IMMORTALITY_ELEGIBLE, 0);
            }
        }
    }

    Creature* SelectRandomFriendlyMissingBuff(uint32 spell)
    {
        std::list<Creature*> lst = DoFindFriendlyMissingBuff(40.0f, spell);
        std::list<Creature*>::const_iterator itr = lst.begin();
        if (lst.empty())
            return NULL;
        advance(itr, rand()%lst.size());
        return (*itr);
    }

    Unit* SelectEnemyCaster(bool /*casting*/)
    {
        std::list<HostileReference*> const& tList = me->getThreatManager().getThreatList();
        std::list<HostileReference*>::const_iterator iter;
        Unit* target;
        for (iter = tList.begin(); iter!=tList.end(); ++iter)
        {
            target = Unit::GetUnit((*me), (*iter)->getUnitGuid());
            if (target && target->getPowerType() == POWER_MANA)
                return target;
        }
        return NULL;
    }

    uint32 EnemiesInRange(float distance)
    {
        std::list<HostileReference*> const& tList = me->getThreatManager().getThreatList();
        std::list<HostileReference*>::const_iterator iter;
        uint32 count = 0;
        Unit* target;
        for (iter = tList.begin(); iter!=tList.end(); ++iter)
        {
            target = Unit::GetUnit((*me), (*iter)->getUnitGuid());
                if (target && me->GetDistance2d(target) < distance)
                    ++count;
        }
        return count;
    }

    void AttackStart(Unit* who)
    {
        if (!who) return;

        if (me->Attack(who, true))
        {
            me->AddThreat(who, 10.0f);
            me->SetInCombatWith(who);
            who->SetInCombatWith(me);

            if (mAIType == AI_MELEE || mAIType == AI_PET)
                DoStartMovement(who);
            else
                DoStartMovement(who, 20.0f);
            SetCombatMovement(true);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (ThreatTimer < uiDiff)
        {
            UpdatePower();
            UpdateThreat();
            ThreatTimer = 4000;
        }
        else ThreatTimer -= uiDiff;

        if (mAIType != AI_PET)
        {
            if (CCTimer < uiDiff)
            {
                RemoveCC();
                CCTimer = 8000+rand()%2000;
            }
            else CCTimer -= uiDiff;
        }

        if (mAIType == AI_MELEE || mAIType == AI_PET) DoMeleeAttackIfReady();
    }
};

/********************************************************************
                            HEALERS
********************************************************************/
enum eDruidSpells
{
    SPELL_LIFEBLOOM_10_N         = 66093,
    SPELL_LIFEBLOOM_25_N         = 67957,
    SPELL_LIFEBLOOM_10_H         = 67958,
    SPELL_LIFEBLOOM_25_H         = 67959,
    SPELL_NOURISH_10_N           = 66066,
    SPELL_NOURISH_25_N           = 67965,
    SPELL_NOURISH_10_H           = 67966,
    SPELL_NOURISH_25_H           = 67967,
    SPELL_REGROWTH_10_N          = 66067,
    SPELL_REGROWTH_25_N          = 67968,
    SPELL_REGROWTH_10_H          = 67969,
    SPELL_REGROWTH_25_H          = 67970,
    SPELL_REJUVENATION_10_N      = 66065,
    SPELL_REJUVENATION_25_N      = 67971,
    SPELL_REJUVENATION_10_H      = 67972,
    SPELL_REJUVENATION_25_H      = 67973,
    SPELL_TRANQUILITY_10_N       = 66086,
    SPELL_TRANQUILITY_25_N       = 67974,
    SPELL_TRANQUILITY_10_H       = 67975,
    SPELL_TRANQUILITY_25_H       = 67976,
    SPELL_BARKSKIN          = 65860, //1 min cd
    SPELL_THORNS            = 66068,
    SPELL_NATURE_GRASP      = 66071, //1 min cd, self buff
};

class mob_toc_druid : public CreatureScript
{
public:
    mob_toc_druid() : CreatureScript("mob_toc_druid") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_druidAI (creature);
    }

    struct mob_toc_druidAI : public boss_faction_championsAI
    {
        mob_toc_druidAI(Creature* creature) : boss_faction_championsAI(creature, AI_HEALER)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiNatureGraspTimer;
        uint32 m_uiTranquilityTimer;
        uint32 m_uiBarkskinTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiNatureGraspTimer = IN_MILLISECONDS;
            m_uiTranquilityTimer = IN_MILLISECONDS;
            m_uiBarkskinTimer = IN_MILLISECONDS;
            m_uiCommonTimer = IN_MILLISECONDS;
            SetEquipmentSlots(false, 51799, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
	        trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
	                {
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiNatureGraspTimer <= uiDiff)
            {
                DoCast(me, SPELL_NATURE_GRASP);
                m_uiNatureGraspTimer = urand(40*IN_MILLISECONDS, 80*IN_MILLISECONDS);
            } else m_uiNatureGraspTimer -= uiDiff;

            if (m_uiTranquilityTimer <= uiDiff)
            {
                DoCastAOE(RAID_MODE(SPELL_TRANQUILITY_10_N, SPELL_TRANQUILITY_25_N, SPELL_TRANQUILITY_10_H, SPELL_TRANQUILITY_25_H));
                m_uiTranquilityTimer = urand(40*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiTranquilityTimer -= uiDiff;

            if (m_uiBarkskinTimer <= uiDiff)
            {
                if (HealthBelowPct(50))
                    DoCast(me, SPELL_BARKSKIN);
                m_uiBarkskinTimer = urand(45*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiBarkskinTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                Unit* phTarget = urand(0, 1) ? me : DoSelectLowestHpFriendly(40.0f);
                if(!phTarget)
                    phTarget = me;

                switch (urand(0, 4))
                {
                    case 0:
                        DoCast(phTarget, RAID_MODE(SPELL_LIFEBLOOM_10_N, SPELL_LIFEBLOOM_25_N, SPELL_LIFEBLOOM_10_H, SPELL_LIFEBLOOM_25_H));
                        break;
                    case 1:
                        DoCast(phTarget, RAID_MODE(SPELL_NOURISH_10_N, SPELL_NOURISH_25_N, SPELL_NOURISH_10_H, SPELL_NOURISH_25_H));
                        break;
                    case 2:
                        DoCast(phTarget, RAID_MODE(SPELL_REGROWTH_10_N, SPELL_REGROWTH_25_N, SPELL_REGROWTH_10_H, SPELL_REGROWTH_25_H));
                        break;
                    case 3:
                        DoCast(phTarget, RAID_MODE(SPELL_REJUVENATION_10_N, SPELL_REJUVENATION_25_N, SPELL_REJUVENATION_10_H, SPELL_REJUVENATION_25_H));
                        break;
                    case 4:
                        if (Creature* target = SelectRandomFriendlyMissingBuff(SPELL_THORNS))
                            DoCast(target, SPELL_THORNS);
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eShamanSpells
{
    SPELL_HEALING_WAVE_10_N          = 66055,
    SPELL_HEALING_WAVE_25_N          = 68115,
    SPELL_HEALING_WAVE_10_H          = 68116,
    SPELL_HEALING_WAVE_25_H          = 68117,
    SPELL_RIPTIDE_10_N               = 66053,
    SPELL_RIPTIDE_25_N               = 68118,
    SPELL_RIPTIDE_10_H               = 68119,
    SPELL_RIPTIDE_25_H               = 68120,
    SPELL_SPIRIT_CLEANSE        = 66056, //friendly only
    SPELL_HEROISM               = 65983,
    SPELL_BLOODLUST             = 65980,
    SPELL_HEX                   = 66054,
    SPELL_EARTH_SHIELD          = 66063,
    SPELL_EARTH_SHOCK_10_N           = 65973,
    SPELL_EARTH_SHOCK_25_N           = 68100,
    SPELL_EARTH_SHOCK_10_H           = 68101,
    SPELL_EARTH_SHOCK_25_H           = 68102,
    AURA_EXHAUSTION             = 57723,
    AURA_SATED                  = 57724,
};

class mob_toc_shaman : public CreatureScript
{
public:
    mob_toc_shaman() : CreatureScript("mob_toc_shaman") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_shamanAI (creature);
    }

    struct mob_toc_shamanAI : public boss_faction_championsAI
    {
        mob_toc_shamanAI(Creature* creature) : boss_faction_championsAI(creature, AI_HEALER)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiHeroismOrBloodlustTimer;
        uint32 m_uiHexTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiHeroismOrBloodlustTimer = IN_MILLISECONDS;
            m_uiHexTimer = IN_MILLISECONDS;
            m_uiCommonTimer = IN_MILLISECONDS;
            SetEquipmentSlots(false, 49992, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiHeroismOrBloodlustTimer <= uiDiff)
            {
                if (me->getFaction()) //Am i alliance?
                {
                    if (!me->HasAura(AURA_EXHAUSTION))
                        DoCastAOE(SPELL_HEROISM);
                    if(!me->HasAura(AURA_EXHAUSTION))
                        DoCastAOE(AURA_EXHAUSTION);
                }
                else
                {
                    if (!me->HasAura(AURA_SATED))
                        DoCastAOE(SPELL_BLOODLUST);
                    if(!me->HasAura(AURA_SATED))
                        DoCastAOE(AURA_SATED);
                }
                m_uiHeroismOrBloodlustTimer = 300*IN_MILLISECONDS;
            } else m_uiHeroismOrBloodlustTimer -= uiDiff;

            if (m_uiHexTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_HEX);
                m_uiHexTimer = urand(10*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiHexTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                Unit* phTarget = urand(0, 1) ? me : DoSelectLowestHpFriendly(40.0f);
                if(!phTarget)
                    phTarget = me;

                switch (urand(0, 5))
                {
                    case 0: case 1:
                        DoCast(phTarget, RAID_MODE(SPELL_HEALING_WAVE_10_N, SPELL_HEALING_WAVE_25_N, SPELL_HEALING_WAVE_10_H, SPELL_HEALING_WAVE_25_H));
                        break;
                    case 2:
                        DoCast(phTarget, RAID_MODE(SPELL_RIPTIDE_10_N, SPELL_RIPTIDE_25_N, SPELL_RIPTIDE_10_H, SPELL_RIPTIDE_25_H));
                        break;
                    case 3:
                        DoCastVictim(RAID_MODE(SPELL_EARTH_SHOCK_10_N, SPELL_EARTH_SHOCK_25_N, SPELL_EARTH_SHOCK_10_H, SPELL_EARTH_SHOCK_25_H));
                        break;
                    case 4:
                        DoCast(phTarget, SPELL_SPIRIT_CLEANSE);
                        break;
                    case 5:
                        if (Unit* target = SelectRandomFriendlyMissingBuff(SPELL_EARTH_SHIELD))
                            DoCast(target, SPELL_EARTH_SHIELD);
                        break;
                }
                m_uiCommonTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum ePaladinSpells
{
    SPELL_HAND_OF_FREEDOM     = 68757, //25 sec cd
    SPELL_BUBBLE              = 66010, //5 min cd
    SPELL_CLEANSE             = 66116,
    SPELL_FLASH_OF_LIGHT_10_N      = 66113,
    SPELL_FLASH_OF_LIGHT_25_N      = 68008,
    SPELL_FLASH_OF_LIGHT_10_H      = 68009,
    SPELL_FLASH_OF_LIGHT_25_H      = 68010,
    SPELL_HOLY_LIGHT_10_N          = 66112,
    SPELL_HOLY_LIGHT_25_N          = 68011,
    SPELL_HOLY_LIGHT_10_H          = 68012,
    SPELL_HOLY_LIGHT_25_H          = 68013,
    SPELL_HOLY_SHOCK_10_N          = 66114,
    SPELL_HOLY_SHOCK_25_N          = 68014,
    SPELL_HOLY_SHOCK_10_H          = 68015,
    SPELL_HOLY_SHOCK_25_H          = 68016,
    SPELL_HAND_OF_PROTECTION  = 66009,
    SPELL_HAMMER_OF_JUSTICE   = 66613,
};

class mob_toc_paladin : public CreatureScript
{
public:
    mob_toc_paladin() : CreatureScript("mob_toc_paladin") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_paladinAI (creature);
    }

    struct mob_toc_paladinAI : public boss_faction_championsAI
    {
        mob_toc_paladinAI(Creature* creature) : boss_faction_championsAI(creature, AI_HEALER)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiBubbleTimer;
        uint32 m_uiHandOfProtectionTimer;
        uint32 m_uiHolyShockTimer;
        uint32 m_uiHandOfFreedomTimer;
        uint32 m_uiHammerOfJusticeTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiBubbleTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            m_uiHandOfProtectionTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            m_uiHolyShockTimer = urand(6*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiHandOfFreedomTimer = urand(25*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            m_uiHammerOfJusticeTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 50771, 47079, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiBubbleTimer <= uiDiff)
            {
                //cast bubble at 20% hp
                if (HealthBelowPct(20))
                    DoCast(me, SPELL_BUBBLE);
                m_uiBubbleTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            } else m_uiBubbleTimer -= uiDiff;

            if (m_uiHandOfProtectionTimer <= uiDiff)
            {
                if (Unit* target = DoSelectLowestHpFriendly(40.0f))
                    if (target->HealthBelowPct(15))
                        DoCast(target, SPELL_HAND_OF_PROTECTION);
                m_uiHandOfProtectionTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            } else m_uiHandOfProtectionTimer -= uiDiff;

            if (m_uiHolyShockTimer <= uiDiff)
            {
                Unit* phTarget;
                switch (urand(0, 2))
                {
                    case 0:
                        phTarget = me;
                        break;
                    case 1:
                        phTarget = DoSelectLowestHpFriendly(40.0f);
                        break;
                    case 2:
                        phTarget = SelectTarget(SELECT_TARGET_RANDOM, 0);
                        break;
                }
                if (phTarget)
                    DoCast(phTarget, RAID_MODE(SPELL_HOLY_SHOCK_10_N, SPELL_HOLY_SHOCK_25_N, SPELL_HOLY_SHOCK_10_H, SPELL_HOLY_SHOCK_25_H));
                m_uiHolyShockTimer = urand(6*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiHolyShockTimer -= uiDiff;

            if (m_uiHandOfFreedomTimer <= uiDiff)
            {
                if (Unit* target = SelectRandomFriendlyMissingBuff(SPELL_HAND_OF_FREEDOM))
                    DoCast(target, SPELL_HAND_OF_FREEDOM);
                m_uiHandOfFreedomTimer = urand(25*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiHandOfFreedomTimer -= uiDiff;

            if (m_uiHammerOfJusticeTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_HAMMER_OF_JUSTICE);
                m_uiHammerOfJusticeTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiHammerOfJusticeTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                Unit* phTarget = urand(0, 1) ? me : DoSelectLowestHpFriendly(40.0f);
                if(!phTarget)
                    phTarget = me;

                switch (urand(0, 4))
                {
                    case 0: case 1:
                        DoCast(phTarget, RAID_MODE(SPELL_FLASH_OF_LIGHT_10_N, SPELL_FLASH_OF_LIGHT_25_N, SPELL_FLASH_OF_LIGHT_10_H, SPELL_FLASH_OF_LIGHT_25_H));
                        break;
                    case 2: case 3:
                        DoCast(phTarget, RAID_MODE(SPELL_HOLY_LIGHT_10_N, SPELL_HOLY_LIGHT_25_N, SPELL_HOLY_LIGHT_10_H, SPELL_HOLY_LIGHT_25_H));
                        break;
                    case 4:
                        DoCast(phTarget, SPELL_CLEANSE);
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum ePriestSpells
{
    SPELL_RENEW_10_N             = 66177,
    SPELL_RENEW_25_N             = 68035,
    SPELL_RENEW_10_H             = 68036,
    SPELL_RENEW_25_H             = 68037,
    SPELL_SHIELD_10_N            = 66099,
    SPELL_SHIELD_25_N            = 68032,
    SPELL_SHIELD_10_H            = 68033,
    SPELL_SHIELD_25_H            = 68034,
    SPELL_FLASH_HEAL_10_N        = 66104,
    SPELL_FLASH_HEAL_25_N        = 68023,
    SPELL_FLASH_HEAL_10_H        = 68024,
    SPELL_FLASH_HEAL_25_H        = 68025,
    SPELL_DISPEL            = 65546,
    SPELL_PSYCHIC_SCREAM    = 65543,
    SPELL_MANA_BURN_10_N         = 66100,
    SPELL_MANA_BURN_25_N         = 68026,
    SPELL_MANA_BURN_10_H         = 68027,
    SPELL_MANA_BURN_25_H         = 68028,
};

class mob_toc_priest : public CreatureScript
{
public:
    mob_toc_priest() : CreatureScript("mob_toc_priest") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_priestAI (creature);
    }

    struct mob_toc_priestAI : public boss_faction_championsAI
    {
        mob_toc_priestAI(Creature* creature) : boss_faction_championsAI(creature, AI_HEALER)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiPsychicScreamTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiPsychicScreamTimer = IN_MILLISECONDS;
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 49992, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiPsychicScreamTimer <= uiDiff)
            {
                if (EnemiesInRange(10.0f) > 2)
                    DoCastAOE(SPELL_PSYCHIC_SCREAM);
                m_uiPsychicScreamTimer = urand(5*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiPsychicScreamTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                Unit* phTarget = urand(0, 1) ? me : DoSelectLowestHpFriendly(40.0f);
                if(!phTarget)
                    phTarget = me;

                switch (urand(0, 5))
                {
                    case 0:
                        DoCast(phTarget, RAID_MODE(SPELL_RENEW_10_N, SPELL_RENEW_25_N, SPELL_RENEW_10_H, SPELL_RENEW_25_H));
                        break;
                    case 1:
                        DoCast(phTarget, RAID_MODE(SPELL_SHIELD_10_N, SPELL_SHIELD_25_N, SPELL_SHIELD_10_H, SPELL_SHIELD_25_H));
                        break;
                    case 2: case 3:
                        DoCast(phTarget, RAID_MODE(SPELL_FLASH_HEAL_10_N, SPELL_FLASH_HEAL_25_N, SPELL_FLASH_HEAL_10_H, SPELL_FLASH_HEAL_25_H));
                        break;
                    case 4:
                        if (Unit* pDTarget = urand(0, 1) ? SelectTarget(SELECT_TARGET_RANDOM, 0) : DoSelectLowestHpFriendly(40.0f))
                            DoCast(pDTarget, SPELL_DISPEL);
                        break;
                    case 5:
                        if (Unit* pBTarget = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(pBTarget, RAID_MODE(SPELL_MANA_BURN_10_N, SPELL_MANA_BURN_25_N, SPELL_MANA_BURN_10_H, SPELL_MANA_BURN_25_H));
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

/********************************************************************
                            RANGED
********************************************************************/
enum eShadowPriestSpells
{
    SPELL_SILENCE           = 65542,
    SPELL_VAMPIRIC_TOUCH_10_N    = 65490,
    SPELL_VAMPIRIC_TOUCH_25_N    = 68091,
    SPELL_VAMPIRIC_TOUCH_10_H    = 68092,
    SPELL_VAMPIRIC_TOUCH_25_H    = 68093,
    SPELL_SW_PAIN_10_N           = 65541,
    SPELL_SW_PAIN_25_N           = 68088,
    SPELL_SW_PAIN_10_H           = 68089,
    SPELL_SW_PAIN_25_H           = 68090,
    SPELL_MIND_FLAY_10_N         = 65488,
    SPELL_MIND_FLAY_25_N         = 68042,
    SPELL_MIND_FLAY_10_H         = 68043,
    SPELL_MIND_FLAY_25_H         = 68044,
    SPELL_MIND_BLAST_10_N        = 65492,
    SPELL_MIND_BLAST_25_N        = 68038,
    SPELL_MIND_BLAST_10_H        = 68039,
    SPELL_MIND_BLAST_25_H        = 68040,
    SPELL_HORROR            = 65545,
    SPELL_DISPERSION        = 65544,
    SPELL_SHADOWFORM        = 16592,
};

class mob_toc_shadow_priest : public CreatureScript
{
public:
    mob_toc_shadow_priest() : CreatureScript("mob_toc_shadow_priest") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_shadow_priestAI (creature);
    }

    struct mob_toc_shadow_priestAI : public boss_faction_championsAI
    {
        mob_toc_shadow_priestAI(Creature* creature) : boss_faction_championsAI(creature, AI_RANGED)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiPsychicScreamTimer;
        uint32 m_uiDispersionTimer;
        uint32 m_uiSilenceTimer;
        uint32 m_uiMindBlastTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiPsychicScreamTimer = urand(5*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiDispersionTimer = urand(1*IN_MILLISECONDS, 180*IN_MILLISECONDS);
            m_uiSilenceTimer = urand(8*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiMindBlastTimer = urand(3*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 50040, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            DoCast(me, SPELL_SHADOWFORM);
            trinket_timer = urand(5000, 20000);
        }

        void EnterCombat(Unit* who)
        {
            boss_faction_championsAI::EnterCombat(who);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiPsychicScreamTimer <= uiDiff)
            {
                if (EnemiesInRange(10.0f) > 2)
                    DoCastAOE(SPELL_PSYCHIC_SCREAM);
                m_uiPsychicScreamTimer = urand(5*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiPsychicScreamTimer -= uiDiff;

            if (m_uiDispersionTimer <= uiDiff)
            {
                if (HealthBelowPct(20))
                    DoCast(me, SPELL_DISPERSION);
                m_uiDispersionTimer = urand(1*IN_MILLISECONDS, 180*IN_MILLISECONDS);
            } else m_uiDispersionTimer -= uiDiff;

            if (m_uiSilenceTimer <= uiDiff)
            {
                if (Unit* target = SelectEnemyCaster(false))
                    DoCast(target, SPELL_SILENCE);
                m_uiSilenceTimer = urand(8*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiSilenceTimer -= uiDiff;

            if (m_uiMindBlastTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, RAID_MODE(SPELL_MIND_BLAST_10_N, SPELL_MIND_BLAST_25_N, SPELL_MIND_BLAST_10_H, SPELL_MIND_BLAST_25_H));
                m_uiMindBlastTimer = urand(3*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiMindBlastTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                if(!target)
                    return;
                switch (urand(0, 4))
                {
                    case 0: case 1:
                        DoCast(target, RAID_MODE(SPELL_MIND_FLAY_10_N, SPELL_MIND_FLAY_25_N, SPELL_MIND_FLAY_10_H, SPELL_MIND_FLAY_25_H));
                        break;
                    case 2:
                        DoCast(target, RAID_MODE(SPELL_VAMPIRIC_TOUCH_10_N, SPELL_VAMPIRIC_TOUCH_25_N, SPELL_VAMPIRIC_TOUCH_10_H, SPELL_VAMPIRIC_TOUCH_25_H));
                        break;
                   case 3:
                        DoCast(target, RAID_MODE(SPELL_SW_PAIN_10_N, SPELL_SW_PAIN_25_N, SPELL_SW_PAIN_10_H, SPELL_SW_PAIN_25_H));
                        break;
                   case 4:
                        if (Unit* pDTarget = urand(0, 1) ? SelectTarget(SELECT_TARGET_RANDOM, 0) : DoSelectLowestHpFriendly(40.0f))
                            DoCast(pDTarget, SPELL_DISPEL);
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eWarlockSpells
{
    SPELL_HELLFIRE_10_N              = 65816,
    SPELL_HELLFIRE_25_N              = 68145,
    SPELL_HELLFIRE_10_H              = 68146,
    SPELL_HELLFIRE_25_H              = 68147,
    SPELL_CORRUPTION_10_N            = 65810,
    SPELL_CORRUPTION_25_N            = 68133,
    SPELL_CORRUPTION_10_H            = 68134,
    SPELL_CORRUPTION_25_H            = 68135,
    SPELL_CURSE_OF_AGONY_10_N        = 65814,
    SPELL_CURSE_OF_AGONY_25_N        = 68136,
    SPELL_CURSE_OF_AGONY_10_H        = 68137,
    SPELL_CURSE_OF_AGONY_25_H        = 68138,
    SPELL_CURSE_OF_EXHAUSTION   = 65815,
    SPELL_FEAR                  = 65809, //8s
    SPELL_SEARING_PAIN_10_N          = 65819,
    SPELL_SEARING_PAIN_25_N          = 68148,
    SPELL_SEARING_PAIN_10_H          = 68149,
    SPELL_SEARING_PAIN_25_H          = 68150,
    SPELL_SHADOW_BOLT_10_N           = 65821,
    SPELL_SHADOW_BOLT_25_N           = 68151,
    SPELL_SHADOW_BOLT_10_H           = 68152,
    SPELL_SHADOW_BOLT_25_H           = 68153,
    SPELL_UNSTABLE_AFFLICTION_10_N   = 65812,
    SPELL_UNSTABLE_AFFLICTION_25_N   = 68154,
    SPELL_UNSTABLE_AFFLICTION_10_H   = 68155,
    SPELL_UNSTABLE_AFFLICTION_25_H   = 68156,
    SPELL_SUMMON_FELHUNTER      = 67514,
};

class mob_toc_warlock : public CreatureScript
{
public:
    mob_toc_warlock() : CreatureScript("mob_toc_warlock") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_warlockAI (creature);
    }

    struct mob_toc_warlockAI : public boss_faction_championsAI
    {
        mob_toc_warlockAI(Creature* creature) : boss_faction_championsAI(creature, AI_RANGED), Summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        SummonList Summons;

        uint32 m_uiFearTimer;
        uint32 m_uiHellfireTimer;
        uint32 m_uiUnstableAfflictionTimer;
        uint32 m_uiCommonTimer;
        uint32 m_uiSummonPetTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiFearTimer = urand(4*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiHellfireTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            m_uiUnstableAfflictionTimer = urand(2*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 49992, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);

            m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            //DoCast(SPELL_SUMMON_FELHUNTER);
            trinket_timer = urand(5000, 20000);			
            if(Creature * pPet = me->FindNearestCreature(35465, 200.0f))
                pPet->DespawnOrUnsummon();
            if(Creature * pPet = me->SummonCreature(35465, me->GetPositionX() + 3.0f, me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000))
                me->SetMinionGUID(pPet->GetGUID());
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiFearTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_FEAR);
                m_uiFearTimer = urand(4*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiFearTimer -= uiDiff;

            if (m_uiHellfireTimer <= uiDiff)
            {
                if (EnemiesInRange(10.0f) > 2)
                    DoCastAOE(RAID_MODE(SPELL_HELLFIRE_10_N, SPELL_HELLFIRE_25_N, SPELL_HELLFIRE_10_H, SPELL_HELLFIRE_25_H));
                m_uiHellfireTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiHellfireTimer -= uiDiff;

            if (m_uiUnstableAfflictionTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, RAID_MODE(SPELL_UNSTABLE_AFFLICTION_10_N, SPELL_UNSTABLE_AFFLICTION_25_N, SPELL_UNSTABLE_AFFLICTION_10_H, SPELL_UNSTABLE_AFFLICTION_25_H));
                m_uiUnstableAfflictionTimer = urand(2*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            } else m_uiUnstableAfflictionTimer -= uiDiff;

            if (m_uiSummonPetTimer <= uiDiff)
            {
                //DoCast(SPELL_SUMMON_FELHUNTER);
                if(Creature * pPet = me->FindNearestCreature(35465, 200.0f))
                {
                m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
                    return;
                }
                if(Creature * pPet = me->SummonCreature(35465, me->GetPositionX() + 3.0f, me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000))
                    me->SetMinionGUID(pPet->GetGUID());
                m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiSummonPetTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                switch (urand(0, 5))
                {
            case 0: case 1:
                        DoCastVictim(RAID_MODE(SPELL_SHADOW_BOLT_10_N, SPELL_SHADOW_BOLT_25_N, SPELL_SHADOW_BOLT_10_H, SPELL_SHADOW_BOLT_25_H));
                        break;
                    case 2:
                        DoCastVictim(RAID_MODE(SPELL_SEARING_PAIN_10_N, SPELL_SEARING_PAIN_25_N, SPELL_SEARING_PAIN_10_H, SPELL_SEARING_PAIN_25_H));
                        break;
                    case 3:
                        DoCastVictim(RAID_MODE(SPELL_CORRUPTION_10_N, SPELL_CORRUPTION_25_N, SPELL_CORRUPTION_10_H, SPELL_CORRUPTION_25_H));
                        break;
                    case 4:
                        DoCastVictim(RAID_MODE(SPELL_CURSE_OF_AGONY_10_N, SPELL_CURSE_OF_AGONY_25_N, SPELL_CURSE_OF_AGONY_10_H, SPELL_CURSE_OF_AGONY_25_H));
                        break;
                    case 5:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_CURSE_OF_EXHAUSTION);
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;
            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eMageSpells
{
    SPELL_ARCANE_BARRAGE_10_N   = 65799, //3s
    SPELL_ARCANE_BARRAGE_25_N   = 67994,
    SPELL_ARCANE_BARRAGE_10_H   = 67995,
    SPELL_ARCANE_BARRAGE_25_H   = 67996,
    SPELL_ARCANE_BLAST_10_N      = 65791,
    SPELL_ARCANE_BLAST_25_N      = 67997,
    SPELL_ARCANE_BLAST_10_H      = 67998,
    SPELL_ARCANE_BLAST_25_H      = 67999,
    SPELL_ARCANE_EXPLOSION_10_N  = 65800,
    SPELL_ARCANE_EXPLOSION_25_N  = 68000,
    SPELL_ARCANE_EXPLOSION_10_H  = 68001,
    SPELL_ARCANE_EXPLOSION_25_H  = 68002,
    SPELL_BLINK             = 65793, //15s
    SPELL_COUNTERSPELL      = 65790, //24s
    SPELL_FROST_NOVA        = 65792, //25s
    SPELL_FROSTBOLT_10_N         = 65807,
    SPELL_FROSTBOLT_25_N         = 68003,
    SPELL_FROSTBOLT_10_H         = 68004,
    SPELL_FROSTBOLT_25_H         = 68005,
    SPELL_ICE_BLOCK         = 65802, //5min
    SPELL_POLYMORPH         = 65801, //15s
};

class mob_toc_mage : public CreatureScript
{
public:
    mob_toc_mage() : CreatureScript("mob_toc_mage") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_mageAI (creature);
    }

    struct mob_toc_mageAI : public boss_faction_championsAI
    {
        mob_toc_mageAI(Creature* creature) : boss_faction_championsAI(creature, AI_RANGED)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiCounterspellTimer;
        uint32 m_uiBlinkTimer;
        uint32 m_uiIceBlockTimer;
        uint32 m_uiPolymorphTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiCounterspellTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiBlinkTimer = urand(7*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiIceBlockTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            m_uiPolymorphTimer = urand(15*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47524, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiCounterspellTimer <= uiDiff)
            {
                if (Unit* target = SelectEnemyCaster(false))
                    DoCast(target, SPELL_COUNTERSPELL);
                m_uiCounterspellTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiCounterspellTimer -= uiDiff;

            if (m_uiBlinkTimer <= uiDiff)
            {
                if (HealthBelowPct(50) && EnemiesInRange(10.0f) > 3)
                {
                    DoCastAOE(SPELL_FROST_NOVA);
                    DoCast(SPELL_BLINK);
                }
                m_uiBlinkTimer = urand(7*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiBlinkTimer -= uiDiff;

            if (m_uiIceBlockTimer <= uiDiff)
            {
                if (HealthBelowPct(20))
                    DoCast(me, SPELL_ICE_BLOCK);
                m_uiIceBlockTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            } else m_uiIceBlockTimer -= uiDiff;

            if (m_uiPolymorphTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_POLYMORPH);
                m_uiPolymorphTimer = urand(15*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiPolymorphTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                switch (urand(0, 2))
                {
                    case 0:
                        DoCastVictim(RAID_MODE(SPELL_ARCANE_BARRAGE_10_N, SPELL_ARCANE_BARRAGE_25_N, SPELL_ARCANE_BARRAGE_10_H, SPELL_ARCANE_BARRAGE_25_H));
                        break;
                    case 1:
                        DoCastVictim(RAID_MODE(SPELL_ARCANE_BLAST_10_N, SPELL_ARCANE_BLAST_25_N, SPELL_ARCANE_BLAST_10_H, SPELL_ARCANE_BLAST_25_H));
                        break;
                    case 2:
                        DoCastVictim(RAID_MODE(SPELL_FROSTBOLT_10_N, SPELL_FROSTBOLT_25_N, SPELL_FROSTBOLT_10_H, SPELL_FROSTBOLT_25_H));
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eHunterSpells
{
    SPELL_AIMED_SHOT_10_N        = 65883,
    SPELL_AIMED_SHOT_25_N        = 67977,
    SPELL_AIMED_SHOT_10_H        = 67978,
    SPELL_AIMED_SHOT_25_H        = 67979,
    SPELL_DETERRENCE             = 65871, //90s
    SPELL_DISENGAGE              = 65869, //30s
    SPELL_EXPLOSIVE_SHOT_10_N    = 65866,
    SPELL_EXPLOSIVE_SHOT_25_N    = 67983,
    SPELL_EXPLOSIVE_SHOT_10_H    = 67984,
    SPELL_EXPLOSIVE_SHOT_25_H    = 67985,
    SPELL_FROST_TRAP        = 65880, //30s
    SPELL_SHOOT_10_N             = 65868, //1.7s
    SPELL_SHOOT_25_N             = 67988, //1.7s
    SPELL_SHOOT_10_H             = 67989, //1.7s
    SPELL_SHOOT_25_H             = 67990, //1.7s
    SPELL_STEADY_SHOT            = 65867, //3s
    SPELL_WING_CLIP              = 66207, //6s
    SPELL_WYVERN_STING           = 65877, //60s
    SPELL_CALL_PET               = 67777,
};

class mob_toc_hunter : public CreatureScript
{
public:
    mob_toc_hunter() : CreatureScript("mob_toc_hunter") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_hunterAI (creature);
    }

    struct mob_toc_hunterAI : public boss_faction_championsAI
    {
        mob_toc_hunterAI(Creature* creature) : boss_faction_championsAI(creature, AI_RANGED), Summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        SummonList Summons;

        uint32 m_uiDisengageTimer;
        uint32 m_uiDeterrenceTimer;
        uint32 m_uiWyvernStingTimer;
        uint32 m_uiFrostTrapTimer;
        uint32 m_uiWingClipTimer;
        uint32 m_uiCommonTimer;
        uint32 m_uiSummonPetTimer;
        uint32 m_uiShotTimer;
        uint32 trinket_timer;		

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiDisengageTimer = urand(12*IN_MILLISECONDS, 20*IN_MILLISECONDS);
            m_uiDeterrenceTimer = urand(20*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            m_uiWyvernStingTimer = urand(7*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            m_uiFrostTrapTimer = urand(12*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            m_uiWingClipTimer = urand(4*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            m_uiShotTimer = urand(1*IN_MILLISECONDS, 2*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47156, EQUIP_NO_CHANGE, 48711);

            m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            //DoCast(SPELL_CALL_PET);
            trinket_timer = urand(5000, 20000);			
            if(Creature * pPet = me->FindNearestCreature(35610, 200.0f))
                pPet->DespawnOrUnsummon();
            if(Creature * pPet = me->SummonCreature(35610, me->GetPositionX() + 3.0f, me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000))
                me->SetMinionGUID(pPet->GetGUID());
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiDisengageTimer <= uiDiff)
            {
                if (EnemiesInRange(10.0f) > 3)
                    DoCast(SPELL_DISENGAGE);
                m_uiDisengageTimer = urand(12*IN_MILLISECONDS, 20*IN_MILLISECONDS);
            } else m_uiDisengageTimer -= uiDiff;

            if (m_uiDeterrenceTimer <= uiDiff)
            {
                if (HealthBelowPct(20))
                    DoCast(SPELL_DETERRENCE);
                m_uiDeterrenceTimer = urand(20*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            } else m_uiDeterrenceTimer -= uiDiff;

            if (m_uiWyvernStingTimer <= uiDiff)
            {
                DoCastVictim(SPELL_WYVERN_STING);
                m_uiWyvernStingTimer = urand(7*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            } else m_uiWyvernStingTimer -= uiDiff;

            if (m_uiFrostTrapTimer <= uiDiff)
            {
                DoCast(SPELL_FROST_TRAP);
                m_uiFrostTrapTimer = urand(12*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiFrostTrapTimer -= uiDiff;

            if (m_uiWingClipTimer <= uiDiff)
            {
                if (me->GetDistance2d(me->getVictim()) < 5.0f)
                    DoCastVictim(SPELL_WING_CLIP);
                m_uiWingClipTimer = urand(4*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiWingClipTimer -= uiDiff;

            if (m_uiSummonPetTimer <= uiDiff)
            {
                //DoCast(SPELL_CALL_PET);
                if(Creature * pPet = me->FindNearestCreature(35610, 200.0f))
                {
                m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
                    return;
                }
                if(Creature * pPet = me->SummonCreature(35610, me->GetPositionX() + 3.0f, me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN, 1000))
                    me->SetMinionGUID(pPet->GetGUID());
                m_uiSummonPetTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiSummonPetTimer -= uiDiff;

            if (m_uiShotTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_SHOOT_10_N, SPELL_SHOOT_25_N, SPELL_SHOOT_10_H, SPELL_SHOOT_25_H));
                m_uiShotTimer = urand(1*IN_MILLISECONDS, 2*IN_MILLISECONDS);
            } else m_uiShotTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                switch (urand(0, 3))
                {
                    case 0: case 1:
                        DoCastVictim(RAID_MODE(SPELL_SHOOT_10_N, SPELL_SHOOT_25_N, SPELL_SHOOT_10_H ,SPELL_SHOOT_25_H));
                        break;
                    case 2:
                        DoCastVictim(RAID_MODE(SPELL_EXPLOSIVE_SHOT_10_N, SPELL_EXPLOSIVE_SHOT_25_N, SPELL_EXPLOSIVE_SHOT_10_H ,SPELL_EXPLOSIVE_SHOT_25_H));
                        break;
                    case 3:
                        DoCastVictim(RAID_MODE(SPELL_AIMED_SHOT_10_N, SPELL_AIMED_SHOT_25_N, SPELL_AIMED_SHOT_10_H, SPELL_AIMED_SHOT_25_H));
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eBoomkinSpells
{
    SPELL_CYCLONE           = 65859, //6s
    SPELL_ENTANGLING_ROOTS  = 65857, //10s
    SPELL_FAERIE_FIRE       = 65863,
    SPELL_FORCE_OF_NATURE   = 65861, //180s
    SPELL_INSECT_SWARM_10_N      = 65855,
    SPELL_INSECT_SWARM_25_N      = 67941,
    SPELL_INSECT_SWARM_10_H      = 67942,
    SPELL_INSECT_SWARM_25_H      = 67943,
    SPELL_MOONFIRE_10_N          = 65856, //5s
    SPELL_MOONFIRE_25_N          = 67944,
    SPELL_MOONFIRE_10_H          = 67945,
    SPELL_MOONFIRE_25_H          = 67946,
    SPELL_STARFIRE_10_N          = 65854,
    SPELL_STARFIRE_25_N          = 67947,
    SPELL_STARFIRE_10_H          = 67948,
    SPELL_STARFIRE_25_H          = 67949,
    SPELL_WRATH_10_N             = 65862,
    SPELL_WRATH_25_N             = 67951,
    SPELL_WRATH_10_H             = 67952,
    SPELL_WRATH_25_H             = 67953,
};

class mob_toc_boomkin : public CreatureScript
{
public:
    mob_toc_boomkin() : CreatureScript("mob_toc_boomkin") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_boomkinAI (creature);
    }

    struct mob_toc_boomkinAI : public boss_faction_championsAI
    {
        mob_toc_boomkinAI(Creature* creature) : boss_faction_championsAI(creature, AI_RANGED)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiBarkskinTimer;
        uint32 m_uiCycloneTimer;
        uint32 m_uiEntanglingRootsTimer;
        uint32 m_uiFaerieFireTimer;
        uint32 m_uiCommonTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiBarkskinTimer = urand(5*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            m_uiCycloneTimer = urand(5*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            m_uiEntanglingRootsTimer = urand(5*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            m_uiFaerieFireTimer = urand(10*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            SetEquipmentSlots(false, 50966, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiBarkskinTimer <= uiDiff)
            {
                if (HealthBelowPct(50))
                    DoCast(me, SPELL_BARKSKIN);
                m_uiBarkskinTimer = urand(5*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            } else m_uiBarkskinTimer -= uiDiff;

            if (m_uiCycloneTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_CYCLONE);
                m_uiCycloneTimer = urand(5*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiCycloneTimer -= uiDiff;

            if (m_uiEntanglingRootsTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_ENTANGLING_ROOTS);
                m_uiEntanglingRootsTimer = urand(5*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiEntanglingRootsTimer -= uiDiff;

            if (m_uiFaerieFireTimer <= uiDiff)
            {
                DoCastVictim(SPELL_FAERIE_FIRE);
                m_uiFaerieFireTimer = urand(10*IN_MILLISECONDS, 40*IN_MILLISECONDS);
            } else m_uiFaerieFireTimer -= uiDiff;

            if (m_uiCommonTimer <= uiDiff)
            {
                switch (urand(0, 6))
                {
                    case 0: case 1:
                        DoCastVictim(RAID_MODE(SPELL_MOONFIRE_10_N, SPELL_MOONFIRE_25_N, SPELL_MOONFIRE_10_H, SPELL_MOONFIRE_25_H));
                        break;
                    case 2:
                        DoCastVictim(RAID_MODE(SPELL_INSECT_SWARM_10_N, SPELL_INSECT_SWARM_25_N, SPELL_INSECT_SWARM_10_H, SPELL_INSECT_SWARM_25_H));
                        break;
                    case 3:
                        DoCastVictim(RAID_MODE(SPELL_STARFIRE_10_N, SPELL_STARFIRE_25_N, SPELL_STARFIRE_10_H, SPELL_STARFIRE_25_H));
                        break;
                    case 4: case 5: case 6:
                        DoCastVictim(RAID_MODE(SPELL_WRATH_10_N, SPELL_WRATH_25_N, SPELL_WRATH_10_H, SPELL_WRATH_25_H));
                        break;
                }
                m_uiCommonTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiCommonTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

/********************************************************************
                            MELEE
********************************************************************/
enum eWarriorSpells
{
    SPELL_BLADESTORM            = 65947,
    SPELL_INTIMIDATING_SHOUT    = 65930,
    SPELL_MORTAL_STRIKE_10_N         = 65926,
    SPELL_MORTAL_STRIKE_25_N         = 68782,
    SPELL_MORTAL_STRIKE_10_H         = 68783,
    SPELL_MORTAL_STRIKE_25_H         = 68784,
    SPELL_CHARGE                = 68764,
    SPELL_DISARM                = 65935,
    SPELL_OVERPOWER             = 65924,
    SPELL_SUNDER_ARMOR          = 65936,
    SPELL_SHATTERING_THROW      = 65940,
    SPELL_RETALIATION           = 65932,
};

class mob_toc_warrior : public CreatureScript
{
public:
    mob_toc_warrior() : CreatureScript("mob_toc_warrior") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_warriorAI (creature);
    }

    struct mob_toc_warriorAI : public boss_faction_championsAI
    {
        mob_toc_warriorAI(Creature* creature) : boss_faction_championsAI(creature, AI_MELEE)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiBladestormTimer;
        uint32 m_uiIntimidatingShoutTimer;
        uint32 m_uiMortalStrikeTimer;
        uint32 m_uiSunderArmorTimer;
        uint32 m_uiChargeTimer;
        uint32 m_uiRetaliationTimer;
        uint32 m_uiOverpowerTimer;
        uint32 m_uiShatteringThrowTimer;
        uint32 m_uiDisarmTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiBladestormTimer = urand(20*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            m_uiIntimidatingShoutTimer = urand(10*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            m_uiMortalStrikeTimer = urand(6*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiSunderArmorTimer = urand(5*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiChargeTimer = urand(3*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiRetaliationTimer = urand(30*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            m_uiOverpowerTimer = urand(30*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            m_uiShatteringThrowTimer = urand(10*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            m_uiDisarmTimer = urand(20*IN_MILLISECONDS, 80*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47427, 46964, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiBladestormTimer <= uiDiff)
            {
                DoCastVictim(SPELL_BLADESTORM);
                m_uiBladestormTimer = urand(20*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiBladestormTimer -= uiDiff;

            if (m_uiIntimidatingShoutTimer <= uiDiff)
            {
                DoCast(me, SPELL_INTIMIDATING_SHOUT);
                m_uiIntimidatingShoutTimer = urand(10*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            } else m_uiIntimidatingShoutTimer -= uiDiff;

            if (m_uiMortalStrikeTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_MORTAL_STRIKE_10_N, SPELL_MORTAL_STRIKE_25_N, SPELL_MORTAL_STRIKE_10_H, SPELL_MORTAL_STRIKE_25_H));
                m_uiMortalStrikeTimer = urand(6*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiMortalStrikeTimer -= uiDiff;

            if (m_uiSunderArmorTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SUNDER_ARMOR);
                m_uiSunderArmorTimer = urand(5*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiSunderArmorTimer -= uiDiff;

            if (m_uiChargeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHARGE);
                m_uiChargeTimer = urand(3*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiChargeTimer -= uiDiff;

            if (m_uiRetaliationTimer <= uiDiff)
            {
                DoCastVictim(SPELL_RETALIATION);
                m_uiRetaliationTimer = urand(30*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            } else m_uiRetaliationTimer -= uiDiff;

            if (m_uiOverpowerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_OVERPOWER);
                m_uiOverpowerTimer = urand(30*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiOverpowerTimer -= uiDiff;

            if (m_uiShatteringThrowTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHATTERING_THROW);
                m_uiShatteringThrowTimer = urand(10*IN_MILLISECONDS, 25*IN_MILLISECONDS);
            } else m_uiShatteringThrowTimer -= uiDiff;

            if (m_uiDisarmTimer <= uiDiff)
            {
                DoCastVictim(SPELL_DISARM);
                m_uiDisarmTimer = urand(20*IN_MILLISECONDS, 80*IN_MILLISECONDS);
            } else m_uiDisarmTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eDeathKnightSpells
{
    SPELL_CHAINS_OF_ICE       = 66020, //8sec
    SPELL_DEATH_COIL_10_N          = 66019, //5sec
    SPELL_DEATH_COIL_25_N          = 67929,
    SPELL_DEATH_COIL_10_H          = 67930,
    SPELL_DEATH_COIL_25_H          = 67931,
    SPELL_DEATH_GRIP          = 66017, //35sec
    SPELL_FROST_STRIKE_10_N        = 66047, //6sec
    SPELL_FROST_STRIKE_25_N        = 67935,
    SPELL_FROST_STRIKE_10_H        = 67936,
    SPELL_FROST_STRIKE_25_H        = 67937,
    SPELL_ICEBOUND_FORTITUDE  = 66023, //1min
    SPELL_ICY_TOUCH_10_N           = 66021, //8sec
    SPELL_ICY_TOUCH_25_N           = 67938,
    SPELL_ICY_TOUCH_10_H           = 67939,
    SPELL_ICY_TOUCH_25_H           = 67940,
    SPELL_STRANGULATE         = 66018, //2min
};

class mob_toc_dk : public CreatureScript
{
public:
    mob_toc_dk() : CreatureScript("mob_toc_dk") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_dkAI (creature);
    }

    struct mob_toc_dkAI : public boss_faction_championsAI
    {
        mob_toc_dkAI(Creature* creature) : boss_faction_championsAI(creature, AI_MELEE)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiIceboundFortitudeTimer;
        uint32 m_uiChainsOfIceTimer;
        uint32 m_uiDeathCoilTimer;
        uint32 m_uiStrangulateTimer;
        uint32 m_uiFrostStrikeTimer;
        uint32 m_uiIcyTouchTimer;
        uint32 m_uiDeathGripTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiIceboundFortitudeTimer = urand(5*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            m_uiChainsOfIceTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiDeathCoilTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiStrangulateTimer = urand(10*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            m_uiFrostStrikeTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            m_uiIcyTouchTimer = urand(8*IN_MILLISECONDS, 12*IN_MILLISECONDS);
            m_uiDeathGripTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47518, 51021, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiIceboundFortitudeTimer <= uiDiff)
            {
                if (HealthBelowPct(50))
                    DoCast(me, SPELL_ICEBOUND_FORTITUDE);
                m_uiIceboundFortitudeTimer = urand(5*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiIceboundFortitudeTimer -= uiDiff;

            if (m_uiChainsOfIceTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_CHAINS_OF_ICE);
                m_uiChainsOfIceTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiChainsOfIceTimer -= uiDiff;

            if (m_uiDeathCoilTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_DEATH_COIL_10_N, SPELL_DEATH_COIL_25_N, SPELL_DEATH_COIL_10_H, SPELL_DEATH_COIL_25_H));
                m_uiDeathCoilTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiDeathCoilTimer -= uiDiff;

            if (m_uiStrangulateTimer <= uiDiff)
            {
                if (Unit* target = SelectEnemyCaster(false))
                    DoCast(target, SPELL_STRANGULATE);
                m_uiStrangulateTimer = urand(10*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiStrangulateTimer -= uiDiff;

            if (m_uiFrostStrikeTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_FROST_STRIKE_10_N, SPELL_FROST_STRIKE_25_N, SPELL_FROST_STRIKE_10_H, SPELL_FROST_STRIKE_25_H));
                m_uiFrostStrikeTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiFrostStrikeTimer -= uiDiff;

            if (m_uiIcyTouchTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_ICY_TOUCH_10_N, SPELL_ICY_TOUCH_25_N, SPELL_ICY_TOUCH_10_H, SPELL_ICY_TOUCH_25_H));
                m_uiIcyTouchTimer = urand(8*IN_MILLISECONDS, 12*IN_MILLISECONDS);
            } else m_uiIcyTouchTimer -= uiDiff;

            if (m_uiDeathGripTimer <= uiDiff)
            {
                if (me->IsInRange(me->getVictim(), 10.0f, 30.0f, false))
                    DoCastVictim(SPELL_DEATH_GRIP);
                m_uiDeathGripTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiDeathGripTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eRogueSpells
{
    SPELL_FAN_OF_KNIVES         = 65955, //2sec
    SPELL_BLIND                 = 65960, //2min
    SPELL_CLOAK                 = 65961, //90sec
    SPELL_BLADE_FLURRY          = 65956, //2min
    SPELL_SHADOWSTEP            = 66178, //30sec
    SPELL_HEMORRHAGE            = 65954,
    SPELL_EVISCERATE_10_N       = 65957,
    SPELL_EVISCERATE_25_N       = 68094,
    SPELL_EVISCERATE_10_H       = 68095,
    SPELL_EVISCERATE_25_H       = 68096,
};

class mob_toc_rogue : public CreatureScript
{
public:
    mob_toc_rogue() : CreatureScript("mob_toc_rogue") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_rogueAI (creature);
    }

    struct mob_toc_rogueAI : public boss_faction_championsAI
    {
        mob_toc_rogueAI(Creature* creature) : boss_faction_championsAI(creature, AI_MELEE)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiFanOfKnivesTimer;
        uint32 m_uiHemorrhageTimer;
        uint32 m_uiEviscerateTimer;
        uint32 m_uiShadowstepTimer;
        uint32 m_uiBlindTimer;
        uint32 m_uiCloakTimer;
        uint32 m_uiBladeFlurryTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiFanOfKnivesTimer = urand(8*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            m_uiHemorrhageTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiEviscerateTimer = urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS);
            m_uiShadowstepTimer = urand(10*IN_MILLISECONDS, 80*IN_MILLISECONDS);
            m_uiBlindTimer = urand(7*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiCloakTimer = urand(20*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            m_uiBladeFlurryTimer = urand(12*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47422, 49982, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiFanOfKnivesTimer <= uiDiff)
            {
                if (EnemiesInRange(15.0f) > 2)
                    DoCastAOE(SPELL_FAN_OF_KNIVES);
                m_uiFanOfKnivesTimer = urand(8*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            } else m_uiFanOfKnivesTimer -= uiDiff;

            if (m_uiHemorrhageTimer <= uiDiff)
            {
                DoCastVictim(SPELL_HEMORRHAGE);
                m_uiHemorrhageTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiHemorrhageTimer -= uiDiff;

            if (m_uiEviscerateTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_EVISCERATE_10_N, SPELL_EVISCERATE_25_N, SPELL_EVISCERATE_10_H, SPELL_EVISCERATE_25_H));
                m_uiEviscerateTimer = urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS);
            } else m_uiEviscerateTimer -= uiDiff;

            if (m_uiShadowstepTimer <= uiDiff)
            {
                if (me->IsInRange(me->getVictim(), 10.0f, 40.0f))
                    DoCastVictim(SPELL_SHADOWSTEP);
                m_uiShadowstepTimer = urand(10*IN_MILLISECONDS, 80*IN_MILLISECONDS);
            } else m_uiShadowstepTimer -= uiDiff;

            if (m_uiBlindTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                    if (me->IsInRange(target, 0.0f, 15.0f, false))
                        DoCast(target, SPELL_BLIND);
                m_uiBlindTimer = urand(7*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiBlindTimer -= uiDiff;

            if (m_uiCloakTimer <= uiDiff)
            {
                if (HealthBelowPct(50))
                    DoCast(me, SPELL_CLOAK);
                m_uiCloakTimer = urand(20*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            } else m_uiCloakTimer -= uiDiff;

            if (m_uiBladeFlurryTimer <= uiDiff)
            {
                DoCast(me, SPELL_BLADE_FLURRY);
                m_uiBladeFlurryTimer = urand(12*IN_MILLISECONDS, 120*IN_MILLISECONDS);
            } else m_uiBladeFlurryTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eEnhShamanSpells
{
    SPELL_EARTH_SHOCK_ENH_10_N   = 65973,
    SPELL_EARTH_SHOCK_ENH_25_N   = 68100,
    SPELL_EARTH_SHOCK_ENH_10_H   = 68101,
    SPELL_EARTH_SHOCK_ENH_25_H   = 68102,
    SPELL_LAVA_LASH              = 65974,
    SPELL_STORMSTRIKE            = 65970,
};

class mob_toc_enh_shaman : public CreatureScript
{
public:
    mob_toc_enh_shaman() : CreatureScript("mob_toc_enh_shaman") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_enh_shamanAI (creature);
    }

    struct mob_toc_enh_shamanAI : public boss_faction_championsAI
    {
        mob_toc_enh_shamanAI(Creature* creature) : boss_faction_championsAI(creature, AI_MELEE), Summons(me)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        SummonList Summons;

        uint32 m_uiHeroismOrBloodlustTimer;
        uint32 m_uiEarthShockTimer;
        uint32 m_uiStormstrikeTimer;
        uint32 m_uiLavaLashTimer;
        uint32 m_uiDeployTotemTimer;
        uint8  m_uiTotemCount;
        float  m_fTotemOldCenterX, m_fTotemOldCenterY;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiHeroismOrBloodlustTimer = urand(25*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            m_uiEarthShockTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiStormstrikeTimer = urand(5*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            m_uiLavaLashTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            m_uiDeployTotemTimer = urand(1*IN_MILLISECONDS, 3*IN_MILLISECONDS);
            m_uiTotemCount = 0;
            m_fTotemOldCenterX = me->GetPositionX();
            m_fTotemOldCenterY = me->GetPositionY();
            SetEquipmentSlots(false, 51803, 48013, EQUIP_NO_CHANGE);
            Summons.DespawnAll();
            trinket_timer = urand(5000, 20000);
        }

        void JustSummoned(Creature* summoned)
        {
            Summons.Summon(summoned);
        }

        void SummonedCreatureDespawn(Creature* /*pSummoned*/)
        {
            --m_uiTotemCount;
        }

        void DeployTotem()
        {
            m_uiTotemCount = 4;
            m_fTotemOldCenterX = me->GetPositionX();
            m_fTotemOldCenterY = me->GetPositionY();
        }

        void JustDied(Unit* killer)
        {
            boss_faction_championsAI::JustDied(killer);
            Summons.DespawnAll();
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiHeroismOrBloodlustTimer <= uiDiff)
            {
                if (me->getFaction()) //Am i alliance?
                {
                    if (!me->HasAura(AURA_EXHAUSTION))
                        DoCastAOE(SPELL_HEROISM);
                    if(!me->HasAura(AURA_EXHAUSTION))
                        DoCastAOE(AURA_EXHAUSTION);
                }
                else
                {
                    if (!me->HasAura(AURA_SATED))
                        DoCastAOE(SPELL_BLOODLUST);
                    if(!me->HasAura(AURA_SATED))
                        DoCastAOE(AURA_SATED);
                }
                m_uiHeroismOrBloodlustTimer = urand(25*IN_MILLISECONDS, 60*IN_MILLISECONDS);
            } else m_uiHeroismOrBloodlustTimer -= uiDiff;

            if (m_uiEarthShockTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_EARTH_SHOCK_ENH_10_N, SPELL_EARTH_SHOCK_ENH_25_N, SPELL_EARTH_SHOCK_ENH_10_H, SPELL_EARTH_SHOCK_ENH_25_H));
                m_uiEarthShockTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiEarthShockTimer -= uiDiff;

            if (m_uiStormstrikeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_STORMSTRIKE);
                m_uiStormstrikeTimer = urand(5*IN_MILLISECONDS, 90*IN_MILLISECONDS);
            } else m_uiStormstrikeTimer -= uiDiff;

            if (m_uiLavaLashTimer <= uiDiff)
            {
                DoCastVictim(SPELL_LAVA_LASH);
                m_uiLavaLashTimer = urand(5*IN_MILLISECONDS, 8*IN_MILLISECONDS);
            } else m_uiLavaLashTimer -= uiDiff;

            if (m_uiDeployTotemTimer <= uiDiff)
            {
                if (m_uiTotemCount < 4 || me->GetDistance2d(m_fTotemOldCenterX, m_fTotemOldCenterY) > 20.0f)
                    DeployTotem();
                m_uiDeployTotemTimer = urand(1*IN_MILLISECONDS, 3*IN_MILLISECONDS);
            } else m_uiDeployTotemTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eRetroPaladinSpells
{
    SPELL_AVENGING_WRATH             = 66011, //3min cd
    SPELL_CRUSADER_STRIKE            = 66003, //6sec cd
    SPELL_DIVINE_SHIELD              = 66010, //5min cd
    SPELL_DIVINE_STORM               = 66006, //10sec cd
    SPELL_HAMMER_OF_JUSTICE_RET      = 66007, //40sec cd
    SPELL_HAND_OF_PROTECTION_RET     = 66009, //5min cd
    SPELL_JUDGEMENT_OF_COMMAND_10_N  = 66005, //8sec cd
    SPELL_JUDGEMENT_OF_COMMAND_25_N  = 68017,
    SPELL_JUDGEMENT_OF_COMMAND_10_H  = 68018,
    SPELL_JUDGEMENT_OF_COMMAND_25_H  = 68019,
    SPELL_REPENTANCE                 = 66008, //60sec cd
    SPELL_SEAL_OF_COMMAND_10_N       = 66004, //no cd
    SPELL_SEAL_OF_COMMAND_25_N       = 68020,
    SPELL_SEAL_OF_COMMAND_10_H       = 68021,
    SPELL_SEAL_OF_COMMAND_25_H       = 68022,
};

class mob_toc_retro_paladin : public CreatureScript
{
public:
    mob_toc_retro_paladin() : CreatureScript("mob_toc_retro_paladin") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_retro_paladinAI (creature);
    }

    struct mob_toc_retro_paladinAI : public boss_faction_championsAI
    {
        mob_toc_retro_paladinAI(Creature* creature) : boss_faction_championsAI(creature, AI_MELEE)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiRepeteanceTimer;
        uint32 m_uiCrusaderStrikeTimer;
        uint32 m_uiAvengingWrathTimer;
        uint32 m_uiDivineShieldTimer;
        uint32 m_uiDivineStormTimer;
        uint32 m_uiJudgementOfCommandTimer;
        uint32 trinket_timer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiRepeteanceTimer = 60*IN_MILLISECONDS;
            m_uiCrusaderStrikeTimer = urand(6*IN_MILLISECONDS, 18*IN_MILLISECONDS);
            m_uiAvengingWrathTimer = 180*IN_MILLISECONDS;
            m_uiDivineShieldTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            m_uiDivineStormTimer = 10*IN_MILLISECONDS;
            m_uiJudgementOfCommandTimer = urand(8*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            SetEquipmentSlots(false, 47519, EQUIP_NO_CHANGE, EQUIP_NO_CHANGE);
            trinket_timer = urand(5000, 20000);
        }

        void EnterCombat(Unit* who)
        {
            boss_faction_championsAI::EnterCombat(who);
            DoCast(me, RAID_MODE(SPELL_SEAL_OF_COMMAND_10_N, SPELL_SEAL_OF_COMMAND_25_N, SPELL_SEAL_OF_COMMAND_10_H, SPELL_SEAL_OF_COMMAND_25_H));
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

                if (IsHeroic())
			{
		if(trinket_timer <= uiDiff)
		{
			if (me->HasAuraType(SPELL_AURA_MOD_STUN) || me->HasAuraType(SPELL_AURA_MOD_FEAR) || me->HasAuraType(SPELL_AURA_MOD_CHARM) || me->HasAuraType(SPELL_AURA_MOD_ROOT) || me->HasAuraType(SPELL_AURA_MOD_CONFUSE))
			{
			    RemoveCC();
				trinket_timer = 120000;
			}
			else 
			{
				trinket_timer= 5000;
			}
		} else trinket_timer -=uiDiff;
			}

            if (m_uiRepeteanceTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_REPENTANCE);
                m_uiRepeteanceTimer = 60*IN_MILLISECONDS;
            } else m_uiRepeteanceTimer -= uiDiff;

            if (m_uiCrusaderStrikeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CRUSADER_STRIKE);
                m_uiCrusaderStrikeTimer = urand(6*IN_MILLISECONDS, 18*IN_MILLISECONDS);
            } else m_uiCrusaderStrikeTimer -= uiDiff;

            if (m_uiAvengingWrathTimer <= uiDiff)
            {
                DoCastVictim(SPELL_AVENGING_WRATH);
                m_uiAvengingWrathTimer = 180*IN_MILLISECONDS;
            } else m_uiAvengingWrathTimer -= uiDiff;

            if (m_uiDivineShieldTimer <= uiDiff)
            {
                if (HealthBelowPct(20))
                    DoCast(me, SPELL_DIVINE_SHIELD);
                m_uiDivineShieldTimer = urand(0*IN_MILLISECONDS, 360*IN_MILLISECONDS);
            } else m_uiDivineShieldTimer -= uiDiff;

            if (m_uiDivineStormTimer <= uiDiff)
            {
                DoCastVictim(SPELL_DIVINE_STORM);
                m_uiDivineStormTimer = 10*IN_MILLISECONDS;
            } else m_uiDivineStormTimer -= uiDiff;

            if (m_uiJudgementOfCommandTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_JUDGEMENT_OF_COMMAND_10_N, SPELL_JUDGEMENT_OF_COMMAND_25_N, SPELL_JUDGEMENT_OF_COMMAND_10_H, SPELL_JUDGEMENT_OF_COMMAND_25_H));
                m_uiJudgementOfCommandTimer = urand(8*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else m_uiJudgementOfCommandTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eWarlockPetSpells
{
    SPELL_DEVOUR_MAGIC  = 67518,
    SPELL_SPELL_LOCK  = 67519,
};

class mob_toc_pet_warlock : public CreatureScript
{
public:
    mob_toc_pet_warlock() : CreatureScript("mob_toc_pet_warlock") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_pet_warlockAI (creature);
    }

    struct mob_toc_pet_warlockAI : public boss_faction_championsAI
    {
        mob_toc_pet_warlockAI(Creature* creature) : boss_faction_championsAI(creature, AI_PET)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiDevourMagicTimer;
        uint32 m_uiSpellLockTimer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiDevourMagicTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            m_uiSpellLockTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

            if (m_uiDevourMagicTimer <= uiDiff)
            {
                DoCastVictim(SPELL_DEVOUR_MAGIC);
                m_uiDevourMagicTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiDevourMagicTimer -= uiDiff;

            if (m_uiSpellLockTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SPELL_LOCK);
                m_uiSpellLockTimer = urand(15*IN_MILLISECONDS, 30*IN_MILLISECONDS);
            } else m_uiSpellLockTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

enum eHunterPetSpells
{
    SPELL_CLAW_10_N  = 67793,
    SPELL_CLAW_25_N  = 67980,
    SPELL_CLAW_10_H  = 67981,
    SPELL_CLAW_25_H  = 67982,
};

class mob_toc_pet_hunter : public CreatureScript
{
public:
    mob_toc_pet_hunter() : CreatureScript("mob_toc_pet_hunter") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_toc_pet_hunterAI (creature);
    }

    struct mob_toc_pet_hunterAI : public boss_faction_championsAI
    {
        mob_toc_pet_hunterAI(Creature* creature) : boss_faction_championsAI(creature, AI_PET)
        {
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
        }

        uint32 m_uiClawTimer;

        void Reset()
        {
            boss_faction_championsAI::Reset();
            m_uiClawTimer = urand(5*IN_MILLISECONDS, 10*IN_MILLISECONDS);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim()) return;

            if (m_uiClawTimer <= uiDiff)
            {
                DoCastVictim(RAID_MODE(SPELL_CLAW_10_N, SPELL_CLAW_25_N, SPELL_CLAW_10_H, SPELL_CLAW_25_H));
                m_uiClawTimer = urand(5*IN_MILLISECONDS, 10*IN_MILLISECONDS);
            } else m_uiClawTimer -= uiDiff;

            boss_faction_championsAI::UpdateAI(uiDiff);
        }
    };
};

/*========================================================*/

void AddSC_boss_faction_champions()
{
    new boss_toc_champion_controller();
    new mob_toc_druid();
    new mob_toc_shaman();
    new mob_toc_paladin();
    new mob_toc_priest();
    new mob_toc_shadow_priest();
    new mob_toc_mage();
    new mob_toc_warlock();
    new mob_toc_hunter();
    new mob_toc_boomkin();
    new mob_toc_warrior();
    new mob_toc_dk();
    new mob_toc_rogue();
    new mob_toc_enh_shaman();
    new mob_toc_retro_paladin();
    new mob_toc_pet_warlock();
    new mob_toc_pet_hunter();
}

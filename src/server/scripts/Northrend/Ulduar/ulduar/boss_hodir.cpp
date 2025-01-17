/*
 * Copyright (C) 2011 ArkCORE <http://www.arkania.net/>
 *
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

/* ScriptData
SDName: Hodir
SD%Complete: 100
SDComments:
EndScriptData */

#include "ScriptPCH.h"
#include "ulduar.h"

enum Spells
{
    // Hodir
    SPELL_FROZEN_BLOWS                          = 62478,
    SPELL_FLASH_FREEZE                          = 61968,
    SPELL_FLASH_FREEZE_VISUAL                   = 62148,
    SPELL_BITING_COLD                           = 62038,
    SPELL_BITING_COLD_TRIGGERED                 = 62039,
    SPELL_BITING_COLD_DAMAGE                    = 62188,
    SPELL_FREEZE                                = 62469,
    SPELL_ICICLE                                = 62234,
    SPELL_ICICLE_SNOWDRIFT                      = 62462,
    SPELL_BLOCK_OF_ICE                          = 61969,
    SPELL_BLOCK_OF_ICE_NPC                      = 61990,
    SPELL_FROZEN_KILL                           = 62226,
    SPELL_ICICLE_FALL                           = 62453,//69428,
    SPELL_FALL_DAMAGE                           = 62236,
    SPELL_FALL_SNOWDRIFT                        = 62460,
    SPELL_BERSERK                               = 47008,
    
    // Druids
    SPELL_WRATH                                 = 62793,
    SPELL_STARLIGHT                             = 62807,
    // Shamans
    SPELL_LAVA_BURST                            = 61924,
    SPELL_STORM_CLOUD                           = 65123,
    // Mages
    SPELL_FIREBALL                              = 61909,
    SPELL_CONJURE_FIRE                          = 62823,
    SPELL_MELT_ICE                              = 64528,
    SPELL_SINGED                                = 62821,
    // Priests
    SPELL_SMITE                                 = 61923,
    SPELL_GREATER_HEAL                          = 62809,
    SPELL_DISPEL_MAGIC                          = 63499
};

// Achievements
#define ACHIEVEMENT_CHEESE_THE_FREEZE           RAID_MODE(2961, 2962)
#define ACHIEVEMENT_COLD_IN_HERE                RAID_MODE(2967, 2968)
#define ACHIEVEMENT_THIS_CACHE_WAS_RARE         RAID_MODE(3182, 3184)
#define ACHIEVEMENT_COOLEST_FRIENDS             RAID_MODE(2963, 2965)

#define ACTION_FAILED_COOLEST_FRIENDS           1

enum Entrys
{
    ENTRY_NPC_FLASH_FREEZE_PRE                        = 32926,
    ENTRY_NPC_FLASH_FREEZE                            = 32938,
    ENTRY_NPC_ICICLE_TARGET                           = 33174,
    ENTRY_NPC_HODIR                                   = 32845
};

enum Events
{
    EVENT_NONE,
    EVENT_FREEZE,
    EVENT_FLASH_CAST,
    EVENT_FLASH_EFFECT,
    EVENT_ICICLE,
    EVENT_BLOWS,
    EVENT_RARE_CACHE,
    EVENT_BERSERK
};

enum Yells
{
    SAY_AGGRO                                   = -1603210,
    SAY_SLAY_1                                  = -1603211,
    SAY_SLAY_2                                  = -1603212,
    SAY_FLASH_FREEZE                            = -1603213,
    SAY_STALACTITE                              = -1603214,
    SAY_DEATH                                   = -1603215,
    SAY_BERSERK                                 = -1603216,
    SAY_YS_HELP                                 = -1603217,
    SAY_HARD_MODE_MISSED                        = -1603218
};

#define EMOTE_FREEZE                            "Hodir begins to cast Flash Freeze!"
#define EMOTE_BLOWS                             "Hodir gains Frozen Blows!"

struct SummonLocation
{
    float x,y,z,o;
    uint32 entry;
};

SummonLocation addLocations[]=
{
    {1983.75f, -243.36f, 432.767f, 1.57f, 32897}, // Priest 1
    {1999.90f, -230.49f, 432.767f, 1.57f, 33325}, // Druid 1
    {2010.06f, -243.45f, 432.767f, 1.57f, 33328}, // Shaman 1
    {2021.12f, -236.65f, 432.767f, 1.57f, 32893}, // Mage 1
    {2028.10f, -244.66f, 432.767f, 1.57f, 33326}, // Priest 2
    {2014.18f, -232.80f, 432.767f, 1.57f, 32901}, // Druid 2
    {1992.90f, -237.54f, 432.767f, 1.57f, 32900}, // Shaman 2
    {1976.60f, -233.53f, 432.767f, 1.57f, 33327}  // Mage 2
};
#define NORMAL_COUNT 4
#define RAID_COUNT 8


class boss_hodir : public CreatureScript
{
public:
    boss_hodir() : CreatureScript("boss_hodir") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_hodirAI(creature);
    }

    struct boss_hodirAI : public BossAI
    {
        boss_hodirAI(Creature* creature) : BossAI(creature, TYPE_HODIR)
        {
            me->ApplySpellImmune(0, IMMUNITY_ID, 65280, true);  // Singed
        }

        uint32 uiCheckIntenseColdTimer;
        bool bMoreThanTwoIntenseCold;
        bool CheeseTheFreeze;
        bool CoolestFriends;
        bool RareCache;

        void Reset()
        {
            _Reset();
        
            me->SetReactState(REACT_PASSIVE);
        
            // Spawn NPC Helpers
            for (uint8 i = 0; i < RAID_MODE<uint8>(NORMAL_COUNT, RAID_COUNT); ++i)
            {
                if (Creature* helper = me->SummonCreature(addLocations[i].entry, addLocations[i].x, addLocations[i].y, addLocations[i].z, addLocations[i].o))
                    if (Creature* iceBlock = helper->SummonCreature(ENTRY_NPC_FLASH_FREEZE_PRE, addLocations[i].x, addLocations[i].y, addLocations[i].z, addLocations[i].o))
                    {
                        helper->AddThreat(me, 5000000.0f);
                        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE<uint32>(64392, 64679), true);
                        me->ApplySpellImmune(0, IMMUNITY_ID, RAID_MODE<uint32>(63525, 63526), true);
                    }
            }
        }

        void KilledUnit(Unit * /*victim*/)
        {
            if (!(rand()%5))
                DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2), me);
        }

        void JustDied(Unit * /*victim*/)
        {
            DoScriptText(SAY_DEATH, me);
        
            me->setFaction(35);
        
            if (instance)
            {
                // Kill credit
                instance->DoUpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET2, 64899);
                // Getting Cold in Here
                if (!bMoreThanTwoIntenseCold)
                    instance->DoCompleteAchievement(ACHIEVEMENT_COLD_IN_HERE);
                // Cheese the Freeze
                if (CheeseTheFreeze)
                    instance->DoCompleteAchievement(ACHIEVEMENT_CHEESE_THE_FREEZE);
                // I Have the Coolest Friends
                if (CoolestFriends)
                    instance->DoCompleteAchievement(ACHIEVEMENT_COOLEST_FRIENDS);
                // I Could Say That This Cache Was Rare
                if (RareCache)
                {
                    instance->DoCompleteAchievement(ACHIEVEMENT_THIS_CACHE_WAS_RARE);
                    instance->SetData(DATA_HODIR_RARE_CHEST, GO_STATE_READY);
                }
            }
            _JustDied();
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();
            DoScriptText(SAY_AGGRO, me);
            me->SetReactState(REACT_AGGRESSIVE);
            DoCast(me, SPELL_BITING_COLD, false);
            events.ScheduleEvent(EVENT_ICICLE, 2000);
            events.ScheduleEvent(EVENT_FREEZE, 25000);
            events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
            events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
            events.ScheduleEvent(EVENT_RARE_CACHE, 180000);
            events.ScheduleEvent(EVENT_BERSERK, 480000);
            uiCheckIntenseColdTimer = 2000;
            bMoreThanTwoIntenseCold = false;
            CheeseTheFreeze = true;
            CoolestFriends = true;
            RareCache = true;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;
            
            if (me->getVictim() && !me->getVictim()->GetCharmerOrOwnerPlayerOrPlayerItself())
                me->Kill(me->getVictim());

            events.Update(diff);
            _DoAggroPulse(diff);
        
            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;
                    
            if (uiCheckIntenseColdTimer < diff && !bMoreThanTwoIntenseCold)
            {
                std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
                for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
                {
                    Unit *target = Unit::GetUnit(*me, (*itr)->getUnitGuid());
                    if (!target || target->GetTypeId() != TYPEID_PLAYER)
                        continue;

                    Aura *AuraIntenseCold = target->GetAura(SPELL_BITING_COLD_TRIGGERED);
                    if (AuraIntenseCold && AuraIntenseCold->GetStackAmount() > 2)
                    {
                        bMoreThanTwoIntenseCold = true;
                        break;
                    }
                }
                uiCheckIntenseColdTimer = 2000;
            } else uiCheckIntenseColdTimer -= diff;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_FREEZE:
                        DoCastAOE(SPELL_FREEZE);
                        events.ScheduleEvent(EVENT_FREEZE, urand(30000, 35000));
                        break;
                    case EVENT_ICICLE:
                        if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            if (target->isAlive())
                                DoCast(target, SPELL_ICICLE);
                        }
                        events.ScheduleEvent(EVENT_ICICLE, 2000);
                        break;
                    case EVENT_FLASH_CAST:
                        DoScriptText(SAY_FLASH_FREEZE, me);
                        me->MonsterTextEmote(EMOTE_FREEZE, 0, true);
                        for (uint8 i = 0; i < RAID_MODE(2,3); ++i)
                        {
                            if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                                if (target->isAlive())
                                    target->CastSpell(target, SPELL_ICICLE_SNOWDRIFT, true);
                        }
                        DoCast(SPELL_FLASH_FREEZE);
                        events.RescheduleEvent(EVENT_ICICLE, 15000);
                        events.ScheduleEvent(EVENT_FLASH_CAST, 50000);
                        events.ScheduleEvent(EVENT_FLASH_EFFECT, 9000);
                        break;
                    case EVENT_FLASH_EFFECT:
                        DoCast(SPELL_FLASH_FREEZE_VISUAL);
                        FlashFreeze();
                        events.CancelEvent(EVENT_FLASH_EFFECT);
                        break;
                    case EVENT_BLOWS:
                        DoScriptText(SAY_STALACTITE, me);
                        me->MonsterTextEmote(EMOTE_BLOWS, 0, true);
                        DoCast(me, SPELL_FROZEN_BLOWS);
                        events.ScheduleEvent(EVENT_BLOWS, urand(60000, 65000));
                        break;
                    case EVENT_RARE_CACHE:
                        DoScriptText(SAY_HARD_MODE_MISSED, me);
                        RareCache = false;
                        events.CancelEvent(EVENT_RARE_CACHE);
                        break;
                    case EVENT_BERSERK:
                        DoCast(me, SPELL_BERSERK, true);
                        DoScriptText(SAY_BERSERK, me);
                        events.CancelEvent(EVENT_BERSERK);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        void FlashFreeze()
        {
            DoZoneInCombat();
            std::list<HostileReference*> ThreatList = me->getThreatManager().getThreatList();
            for (std::list<HostileReference*>::const_iterator itr = ThreatList.begin(); itr != ThreatList.end(); ++itr)
            {
                if (Unit *target = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                {
                    if(!target->ToPlayer())
                        continue;

                    if (target->HasAura(SPELL_BLOCK_OF_ICE))
                    {
                        DoCast(target, SPELL_FROZEN_KILL);
                        continue;
                    }
                    else
                    {
                        if (GetClosestCreatureWithEntry(target, ENTRY_NPC_ICICLE_TARGET, 5.0f))
                            continue;
                        
                        else if (Creature *pIceBlock = target->SummonCreature(ENTRY_NPC_FLASH_FREEZE,target->GetPositionX(),target->GetPositionY(),target->GetPositionZ(),0,TEMPSUMMON_TIMED_DESPAWN,105000))
                        {
                            if (target->GetTypeId() == TYPEID_PLAYER)
                                CheeseTheFreeze = false;
                        }
                    }
                }
            }
        }
    
        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_FAILED_COOLEST_FRIENDS:
                    CoolestFriends = false;
                    break;
            }
        }
    };

};


class npc_icicle : public CreatureScript
{
public:
    npc_icicle() : CreatureScript("npc_icicle") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_icicleAI (creature);
    }

    struct npc_icicleAI : public Scripted_NoMovementAI
    {
        npc_icicleAI(Creature *creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(28470);
        }

        uint32 IcicleTimer;

        void Reset()
        {
            IcicleTimer = 2000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (IcicleTimer <= diff)
            {
                DoCast(me, SPELL_FALL_DAMAGE);
                DoCast(me, SPELL_ICICLE_FALL);
                IcicleTimer = 10000;
            }
            else IcicleTimer -= diff;
        }
    };

};

class npc_icicle_snowdrift : public CreatureScript
{
public:
    npc_icicle_snowdrift() : CreatureScript("npc_icicle_snowdrift") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_icicle_snowdriftAI (creature);
    }

    struct npc_icicle_snowdriftAI : public Scripted_NoMovementAI
    {
        npc_icicle_snowdriftAI(Creature *creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(28470);
        }

        uint32 IcicleTimer;

        void Reset()
        {
            IcicleTimer = 2000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (IcicleTimer <= diff)
            {
                DoCast(me, SPELL_FALL_SNOWDRIFT);
                DoCast(me, SPELL_ICICLE_FALL);
                IcicleTimer = 10000;
            }
            else IcicleTimer -= diff;
        }
    };
};

class npc_snowpacked_icicle : public CreatureScript
{
public:
    npc_snowpacked_icicle() : CreatureScript("npc_snowpacked_icicle") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_snowpacked_icicleAI (creature);
    }

    struct npc_snowpacked_icicleAI : public Scripted_NoMovementAI
    {
        npc_snowpacked_icicleAI(Creature *creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(15880);
        }

        uint32 DespawnTimer;

        void Reset()
        {
            DespawnTimer = 12000;
        }

        void UpdateAI(const uint32 diff)
        {
            if (DespawnTimer <= diff)
            {
                if (GameObject *pSnowdrift = me->FindNearestGameObject(194173, 2))
                    me->RemoveGameObject(pSnowdrift, true);
                me->ForcedDespawn();
            }
            else DespawnTimer -= diff;
        }
    };
};

class npc_hodir_priest : public CreatureScript
{
public:
    npc_hodir_priest() : CreatureScript("npc_hodir_priest") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hodir_priestAI (creature);
    }

    struct npc_hodir_priestAI : public ScriptedAI
    {
        npc_hodir_priestAI(Creature *creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* pInstance;
        uint32 HealTimer;
        uint32 DispelTimer;

        void Reset()
        {
            HealTimer = urand(4000, 8000);
            DispelTimer = urand(20000, 30000);
        }

        void AttackStart(Unit *who)
        {
            AttackStartCaster(who, 20);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_STUNNED))
                return;

            if (HealthBelowPct(35))
                DoCastAOE(SPELL_GREATER_HEAL, true);

            if (HealTimer <= uiDiff)
            {
                DoCastAOE(SPELL_GREATER_HEAL, true);
                HealTimer = urand(12000, 14000);
            }
            else HealTimer -= uiDiff;
        
            if (DispelTimer < uiDiff)
            {
                std::list<Player*> players;
                Arkcore::AnyPlayerInObjectRangeCheck checker(me, 30);
                Arkcore::PlayerListSearcher<Arkcore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                me->VisitNearbyWorldObject(30, searcher);
                if (!players.empty())
                {
                    for (std::list<Player*>::iterator iter = players.begin(); iter != players.end(); ++iter)
                    {
                        if ((*iter)->HasAura(SPELL_FREEZE))
                        {
                            DoCast((*iter),SPELL_DISPEL_MAGIC, true);
                            DispelTimer = urand(25000,30000);
                            return;
                        }
                    }
                }
                DispelTimer = 5000;
            }
            else DispelTimer -= uiDiff;

            DoSpellAttackIfReady(SPELL_SMITE);
        }

        void JustDied(Unit* /*victim*/)
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };
};

class npc_hodir_shaman : public CreatureScript
{
public:
    npc_hodir_shaman() : CreatureScript("npc_hodir_shaman") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hodir_shamanAI (creature);
    }

    struct npc_hodir_shamanAI : public ScriptedAI
    {
        npc_hodir_shamanAI(Creature *creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* pInstance;
        uint32 StormTimer;

        void Reset()
        {
            StormTimer = urand(15000, 20000);
        }

        void AttackStart(Unit *who)
        {
            AttackStartCaster(who, 20);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_STUNNED))
                return;

            if (StormTimer <= uiDiff)
            {
                std::list<Player*> players;
                Arkcore::AnyPlayerInObjectRangeCheck checker(me, 30);
                Arkcore::PlayerListSearcher<Arkcore::AnyPlayerInObjectRangeCheck> searcher(me, players, checker);
                me->VisitNearbyWorldObject(30, searcher);
                if (!players.empty())
                {
                    std::list<Player*>::iterator iter = players.begin(); 
                    DoCast((*iter), SPELL_STORM_CLOUD, true);
                }
                StormTimer = urand(15000, 20000);
            }
            else StormTimer -= uiDiff;

            DoSpellAttackIfReady(SPELL_LAVA_BURST);
        }

        void JustDied(Unit* /*victim*/)
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };
};

class npc_hodir_druid : public CreatureScript
{
public:
    npc_hodir_druid() : CreatureScript("npc_hodir_druid") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hodir_druidAI (creature);
    }

    struct npc_hodir_druidAI : public ScriptedAI
    {
        npc_hodir_druidAI(Creature *creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* pInstance;
        uint32 StarlightTimer;

        void Reset()
        {
            StarlightTimer = urand(10000, 15000);
        }

        void AttackStart(Unit *who)
        {
            AttackStartCaster(who, 20);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_STUNNED))
                return;

            if (StarlightTimer <= uiDiff)
            {
                DoCast(me, SPELL_STARLIGHT, true);
                StarlightTimer = urand(20000, 25000);
            }
            else StarlightTimer -= uiDiff;

            DoSpellAttackIfReady(SPELL_WRATH);
        }

        void JustDied(Unit* /*victim*/)
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };
};

class npc_hodir_mage : public CreatureScript
{
public:
    npc_hodir_mage() : CreatureScript("npc_hodir_mage") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hodir_mageAI (creature);
    }

    struct npc_hodir_mageAI : public ScriptedAI
    {
        npc_hodir_mageAI(Creature *creature) : ScriptedAI(creature)
        {
            pInstance = creature->GetInstanceScript();
            me->setFaction(1665);
        }

        InstanceScript* pInstance;
        uint32 FireTimer;
        uint32 MeltIceTimer;

        void Reset()
        {
            FireTimer = urand(15000, 20000);
            MeltIceTimer = 5000;
        }

        void AttackStart(Unit *who)
        {
            AttackStartCaster(who, 20);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_STUNNED))
                return;

            if (FireTimer <= uiDiff)
            {
                DoCast(me, SPELL_CONJURE_FIRE, true);
                FireTimer = urand(25000, 30000);
            }
            else FireTimer -= uiDiff;
        
            if (MeltIceTimer < uiDiff)
            {
                if (Creature *pShard = me->FindNearestCreature(ENTRY_NPC_FLASH_FREEZE,50,true))
                {
                    DoCast(pShard, SPELL_MELT_ICE, true);
                    MeltIceTimer = urand(5000,10000);
                }
                MeltIceTimer = 5000;
            }
            else MeltIceTimer -= uiDiff;

            DoSpellAttackIfReady(SPELL_FIREBALL);
        }

        void JustDied(Unit* /*victim*/)
        {
            // I Have the Coolest Friends
            if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR,60,true))
                pHodir->AI()->DoAction(ACTION_FAILED_COOLEST_FRIENDS);
        }
    };
};

class npc_toasty_fire : public CreatureScript
{
public:
    npc_toasty_fire() : CreatureScript("npc_toasty_fire") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_toasty_fireAI (creature);
    }

    struct npc_toasty_fireAI : public Scripted_NoMovementAI
    {
        npc_toasty_fireAI(Creature *creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
            me->SetReactState(REACT_PASSIVE);
            me->SetDisplayId(15880);
        }

        void Reset()
        {
            DoCast(me, SPELL_SINGED, true);
        }

        void SpellHit(Unit* /*who*/, SpellInfo const* spell)
        {
            // Toasty fire can be extinguished by falling ice or Flash Freeze
            if (spell->Id == SPELL_BLOCK_OF_ICE || spell->Id == 62457 || spell->Id == 65370)
            {
                if (GameObject *pFire = me->FindNearestGameObject(194300, 4))
                    me->RemoveGameObject(pFire, true);
                me->ForcedDespawn();
            }
        }

        void JustDied(Unit * /*victim*/)
        {
            if (GameObject *pFire = me->FindNearestGameObject(194300, 4))
                me->RemoveGameObject(pFire, true);
        }
    };
};

class npc_flash_freeze : public CreatureScript
{
public:
    npc_flash_freeze() : CreatureScript("npc_flash_freeze") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_flash_freezeAI (creature);
    }

    struct npc_flash_freezeAI : public Scripted_NoMovementAI
    {
        npc_flash_freezeAI(Creature *creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED | UNIT_FLAG_PACIFIED);
            me->setFaction(25865);
            me->setFaction(14);
        }

        void Reset()
        {
            if (me->ToTempSummon()->GetSummoner())
            {
                if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE)
                    DoCast(SPELL_BLOCK_OF_ICE_NPC);
                else
                    DoCast(SPELL_BLOCK_OF_ICE);

                me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!me->ToTempSummon()->GetSummoner())
            {
                me->DisappearAndDie();
                return;
            }

            if (me->ToTempSummon()->GetSummoner()->isDead())
            {
                me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
                me->DisappearAndDie();
                return;
            }

            if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE && !me->ToTempSummon()->GetSummoner()->HasAura(SPELL_BLOCK_OF_ICE_NPC))
                DoCast(SPELL_BLOCK_OF_ICE_NPC);
        }

        void DamageTaken(Unit* killer, uint32 &damage)
        {
            if (me->GetEntry() == ENTRY_NPC_FLASH_FREEZE_PRE)
            {
                if (killer && killer->GetTypeId() == TYPEID_PLAYER)
                    if (Creature* pHodir = me->FindNearestCreature(NPC_HODIR, 80, true))
                        if (!pHodir->isInCombat())
                            pHodir->AI()->AttackStart(killer);
            }
        }

        void JustDied(Unit * /*victim*/)
        {
            if (me->ToTempSummon()->GetSummoner())
                me->ToTempSummon()->GetSummoner()->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, false);
        }
    };
};

class spell_biting_cold : public SpellScriptLoader
{
    public:
        spell_biting_cold() : SpellScriptLoader("spell_biting_cold") { }

    class spell_biting_cold_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_biting_cold_AuraScript)

        void HandlePeriodicDummy(AuraEffect const* aurEff)
        {
            PreventDefaultAction();
            if (Unit* trigger = GetTarget())
            {
                if(aurEff->GetSpellInfo()->Id == SPELL_BITING_COLD)
                {
                    if(trigger->ToPlayer())
                    {
                        if(!trigger->HasAura(62821)) // Not Triggered if in Toasty Fire
                            trigger->CastSpell(trigger,SPELL_BITING_COLD_TRIGGERED,true,0,0,GetCasterGUID());
                    }
                }
                else if(aurEff->GetSpellInfo()->Id == SPELL_BITING_COLD_TRIGGERED)
                {
                    int32 damage = trigger->GetAuraCount(SPELL_BITING_COLD_TRIGGERED);
                    damage *= 400;
                    trigger->CastCustomSpell(trigger,SPELL_BITING_COLD_DAMAGE, &damage, NULL, NULL, true, 0, 0, GetCasterGUID());
                }
            }
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_biting_cold_AuraScript::HandlePeriodicDummy, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        }
    };

    AuraScript *GetAuraScript() const
    {
        return new spell_biting_cold_AuraScript();
    }
};

void AddSC_boss_hodir()
{
    new boss_hodir();
    new npc_icicle();
    new npc_icicle_snowdrift();
    new npc_snowpacked_icicle();
    new npc_hodir_priest();
    new npc_hodir_shaman();
    new npc_hodir_druid();
    new npc_hodir_mage();
    new npc_toasty_fire();
    new npc_flash_freeze();
    new spell_biting_cold();
}

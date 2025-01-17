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
#include "nexus.h"

enum Spells
{
    SPELL_ICE_NOVA                                = 47772,
    H_SPELL_ICE_NOVA                              = 56935,
    SPELL_FIREBOMB                                = 47773,
    H_SPELL_FIREBOMB                              = 56934,
    SPELL_GRAVITY_WELL                            = 47756,
    SPELL_TELESTRA_BACK                           = 47714,
    SPELL_FIRE_MAGUS_VISUAL                       = 47705,
    SPELL_FROST_MAGUS_VISUAL                      = 47706,
    SPELL_ARCANE_MAGUS_VISUAL                     = 47704,
    SPELL_CRITTER                                 = 47731,
    SPELL_TIMESTOP                                = 47736
};

enum Creatures
{
    MOB_FIRE_MAGUS                                = 26928,
    MOB_FROST_MAGUS                               = 26930,
    MOB_ARCANE_MAGUS                              = 26929
};

enum Yells
{
    SAY_AGGRO                                     = -1576000,
    SAY_KILL                                      = -1576001,
    SAY_DEATH                                     = -1576002,
    SAY_MERGE                                     = -1576003,
    SAY_SPLIT_1                                   = -1576004,
    SAY_SPLIT_2                                   = -1576005
};

#define ACTION_MAGUS_DEAD                         1
#define DATA_SPLIT_PERSONALITY                    2

const Position  CenterOfRoom = {504.80f, 89.07f, -16.12f, 6.27f};

class boss_magus_telestra : public CreatureScript
{
public:
    boss_magus_telestra() : CreatureScript("boss_magus_telestra") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_magus_telestraAI (creature);
    }

    struct boss_magus_telestraAI : public ScriptedAI
    {
        boss_magus_telestraAI(Creature* c) : ScriptedAI(c)
        {
            pInstance = c->GetInstanceScript();
        }

        InstanceScript* pInstance;

        uint64 uiFireMagusGUID;
        uint64 uiFrostMagusGUID;
        uint64 uiArcaneMagusGUID;

        bool bFireMagusDead;
        bool bFrostMagusDead;
        bool bArcaneMagusDead;
        bool bIsWaitingToAppear;

        uint32 uiIsWaitingToAppearTimer;
        uint32 uiIceNovaTimer;
        uint32 uiFireBombTimer;
        uint32 uiGravityWellTimer;
        uint32 uiCooldown;

        uint8 Phase;
        uint8 splitPersonality;
        time_t time[3];

        void Reset()
        {
            Phase = 0;
            //These times are probably wrong
            uiIceNovaTimer =  7*IN_MILLISECONDS;
            uiFireBombTimer =  0;
            uiGravityWellTimer = 15*IN_MILLISECONDS;
            uiCooldown = 0;

            uiFireMagusGUID = 0;
            uiFrostMagusGUID = 0;
            uiArcaneMagusGUID = 0;

            for (uint8 n = 0; n < 3; ++n)
                time[n] = 0;
				
            splitPersonality = 0;
            bIsWaitingToAppear = false;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            me->SetVisible(true);

            if (pInstance)
                pInstance->SetData(DATA_MAGUS_TELESTRA_EVENT, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoScriptText(SAY_AGGRO, me);

            if (pInstance)
                pInstance->SetData(DATA_MAGUS_TELESTRA_EVENT, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        {
            DoScriptText(SAY_DEATH, me);

            if (pInstance)
                pInstance->SetData(DATA_MAGUS_TELESTRA_EVENT, DONE);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            DoScriptText(SAY_KILL, me);
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_MAGUS_DEAD)
            {
                uint8 i = 0;
                while (time[i] != 0)
                    ++i;

                time[i] = sWorld->GetGameTime();
                if (i == 2 && (time[2] - time[1] < 5) && (time[1] - time[0] < 5))
                    ++splitPersonality;
            }
        }

        uint32 GetData(uint32 type)
        {
            if (type == DATA_SPLIT_PERSONALITY)
                return splitPersonality;

            return 0;
        }

        uint64 SplitPersonality(uint32 entry)
        {
            if (Creature* Summoned = me->SummonCreature(entry, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1*IN_MILLISECONDS))
            {
                switch (entry)
                {
                    case MOB_FIRE_MAGUS:
                    {
                        Summoned->CastSpell(Summoned, SPELL_FIRE_MAGUS_VISUAL, false);
                        break;
                    }
                    case MOB_FROST_MAGUS:
                    {
                        Summoned->CastSpell(Summoned, SPELL_FROST_MAGUS_VISUAL, false);
                        break;
                    }
                    case MOB_ARCANE_MAGUS:
                    {
                        Summoned->CastSpell(Summoned, SPELL_ARCANE_MAGUS_VISUAL, false);
                        break;
                    }
                }
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    Summoned->AI()->AttackStart(target);
                return Summoned->GetGUID();
            }
            return 0;
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            if (summon->isAlive())
                return;

            if (summon->GetGUID() == uiFireMagusGUID)
            {
                me->AI()->DoAction(ACTION_MAGUS_DEAD);
                bFireMagusDead = true;
            }
            else if (summon->GetGUID() == uiFrostMagusGUID)
            {
                me->AI()->DoAction(ACTION_MAGUS_DEAD);
                bFrostMagusDead = true;
            }
            else if (summon->GetGUID() == uiArcaneMagusGUID)
            {
                me->AI()->DoAction(ACTION_MAGUS_DEAD);
                bArcaneMagusDead = true;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (bIsWaitingToAppear)
            {
                me->StopMoving();
                me->AttackStop();
                if (uiIsWaitingToAppearTimer <= diff)
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    bIsWaitingToAppear = false;
                } else uiIsWaitingToAppearTimer -= diff;
                return;
            }

            if ((Phase == 1) ||(Phase == 3))
            {
                if (bFireMagusDead && bFrostMagusDead && bArcaneMagusDead)
                {
                    for (uint8 n = 0; n < 3; ++n)
                        time[n] = 0;
                    me->GetMotionMaster()->Clear();
                    me->GetMap()->CreatureRelocation(me, CenterOfRoom.GetPositionX(), CenterOfRoom.GetPositionY(), CenterOfRoom.GetPositionZ(), CenterOfRoom.GetOrientation());
                    DoCast(me, SPELL_TELESTRA_BACK);
                    me->SetVisible(true);
                    Phase++;
                    uiFireMagusGUID = 0;
                    uiFrostMagusGUID = 0;
                    uiArcaneMagusGUID = 0;
                    bIsWaitingToAppear = true;
                    uiIsWaitingToAppearTimer = 4*IN_MILLISECONDS;
                    DoScriptText(SAY_MERGE, me);
                }
                else
                    return;
            }

            if ((Phase == 0) && HealthBelowPct(50))
            {
                Phase = 1;
                me->CastStop();
                me->RemoveAllAuras();
                me->SetVisible(false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                uiFireMagusGUID = SplitPersonality(MOB_FIRE_MAGUS);
                uiFrostMagusGUID = SplitPersonality(MOB_FROST_MAGUS);
                uiArcaneMagusGUID = SplitPersonality(MOB_ARCANE_MAGUS);
                bFireMagusDead = false;
                bFrostMagusDead = false;
                bArcaneMagusDead = false;
                DoScriptText(RAND(SAY_SPLIT_1, SAY_SPLIT_2), me);
                return;
            }

        if (IsHeroic() && (Phase == 2) && HealthBelowPct(15))
            {
                Phase = 3;
                me->CastStop();
                me->RemoveAllAuras();
                me->SetVisible(false);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                uiFireMagusGUID = SplitPersonality(MOB_FIRE_MAGUS);
                uiFrostMagusGUID = SplitPersonality(MOB_FROST_MAGUS);
                uiArcaneMagusGUID = SplitPersonality(MOB_ARCANE_MAGUS);
                bFireMagusDead = false;
                bFrostMagusDead = false;
                bArcaneMagusDead = false;
                DoScriptText(RAND(SAY_SPLIT_1, SAY_SPLIT_2), me);
                return;
            }

            if (uiCooldown)
            {
                if (uiCooldown <= diff)
                    uiCooldown = 0;
                else
                {
                    uiCooldown -= diff;
                    return;
                }
            }

            if (uiIceNovaTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    DoCast(target, SPELL_ICE_NOVA, false);
                    uiCooldown = 1500;
                }
                uiIceNovaTimer = 15*IN_MILLISECONDS;
            } else uiIceNovaTimer -= diff;

            if (uiGravityWellTimer <= diff)
            {
                if (Unit* target = me->getVictim())
                {
                    DoCast(target, SPELL_GRAVITY_WELL);
                    uiCooldown = 6*IN_MILLISECONDS;
                }
                uiGravityWellTimer = 15*IN_MILLISECONDS;
            } else uiGravityWellTimer -= diff;

            if (uiFireBombTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    DoCast(target, SPELL_FIREBOMB, false);
                    uiCooldown = 2*IN_MILLISECONDS;
                }
                uiFireBombTimer = 2*IN_MILLISECONDS;
            } else uiFireBombTimer -=diff;

            DoMeleeAttackIfReady();
        }
    };

};	  

class achievement_split_personality : public AchievementCriteriaScript
{
    public:
        achievement_split_personality() : AchievementCriteriaScript("achievement_split_personality")
        {
        }

        bool OnCheck(Player* /*player*/, Unit* target)
        {
            if (!target)
                return false;

            if (Creature* Telestra = target->ToCreature())
                if (Telestra->AI()->GetData(DATA_SPLIT_PERSONALITY) == 2)
                    return true;

            return false;
        }
};

class boss_magus_telestra_arcane : public CreatureScript
{
public:
    boss_magus_telestra_arcane() : CreatureScript("boss_magus_telestra_arcane") { }

    struct boss_magus_telestra_arcaneAI : public ScriptedAI
    {
        boss_magus_telestra_arcaneAI(Creature* c) : ScriptedAI(c)
        {
            pInstance = c->GetInstanceScript();
        }

        InstanceScript* pInstance;
        uint32 uiCritterTimer;
        uint32 uiTimeStopTimer;

        void Reset()
        {
            uiCritterTimer = urand(3000, 6000);
            uiTimeStopTimer = urand(7000, 10000);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (uiCritterTimer<=diff)
            {
                DoCast(SPELL_CRITTER);
                    uiCritterTimer = urand(5000, 8000);
            }else uiCritterTimer-=diff;

            if (uiTimeStopTimer<=diff)
            {
                DoCastAOE(SPELL_TIMESTOP);
                uiTimeStopTimer = urand(15000, 18000);
            } else uiTimeStopTimer-=diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new boss_magus_telestra_arcaneAI(pCreature);
    }
};

class spell_nexus_critter_targeting : public SpellScriptLoader
{
    public:
        spell_nexus_critter_targeting() : SpellScriptLoader("spell_nexus_critter_targeting") { }

        class spell_nexus_critter_targeting_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_nexus_critter_targeting_SpellScript);

            void FilterTargetsInitial(std::list<Unit*>& unitList)
            {
                sharedUnitList = unitList;
            }

            void FilterTargetsSubsequent(std::list<Unit*>& unitList)
            {
                unitList = sharedUnitList;
            }

            void Register()
            {
                OnUnitTargetSelect += SpellUnitTargetFn(spell_nexus_critter_targeting_SpellScript::FilterTargetsInitial, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnUnitTargetSelect += SpellUnitTargetFn(spell_nexus_critter_targeting_SpellScript::FilterTargetsSubsequent, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
                OnUnitTargetSelect += SpellUnitTargetFn(spell_nexus_critter_targeting_SpellScript::FilterTargetsSubsequent, EFFECT_2, TARGET_UNIT_SRC_AREA_ENEMY);
            }

            std::list<Unit*> sharedUnitList;
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_nexus_critter_targeting_SpellScript();
        }
};

void AddSC_boss_magus_telestra()
{
    new boss_magus_telestra();
    new achievement_split_personality();	
    new boss_magus_telestra_arcane();
    new spell_nexus_critter_targeting();
}
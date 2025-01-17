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
    SPELL_CRYSTAL_SPIKES                          = 47958, //doesn't work, using workaround
    H_SPELL_CRYSTAL_SPIKES                        = 57082, //doesn't work, using workaround
    SPELL_CRYSTALL_SPIKE_DAMAGE                   = 47944,
    H_SPELL_CRYSTALL_SPIKE_DAMAGE                 = 57067,
    SPELL_CRYSTAL_SPIKE_PREVISUAL                 = 50442,
    SPELL_SPELL_REFLECTION                        = 47981,
    SPELL_TRAMPLE                                 = 48016,
    H_SPELL_TRAMPLE                               = 57066,
    SPELL_FRENZY                                  = 48017,
    SPELL_SUMMON_CRYSTALLINE_TANGLER              = 61564, //summons npc 32665
    SPELL_TANGLE                                  = 61555
};

enum Yells
{
    SAY_AGGRO                                     = -1576020,
    SAY_DEATH                                     = -1576021,
    SAY_REFLECT                                   = -1576022,
    SAY_CRYSTAL_SPIKES                            = -1576023,
    SAY_KILL                                      = -1576024
};

enum Creatures
{
    MOB_CRYSTAL_SPIKE                             = 27099,
    MOB_CRYSTALLINE_TANGLER                       = 32665
};

#define SPIKE_DISTANCE                            5.0f

class boss_ormorok : public CreatureScript
{
public:
    boss_ormorok() : CreatureScript("boss_ormorok") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ormorokAI (creature);
    }

    struct boss_ormorokAI : public ScriptedAI
    {
        boss_ormorokAI(Creature* c) : ScriptedAI(c)
        {
            pInstance = c->GetInstanceScript();
        }

        InstanceScript* pInstance;

        bool bFrenzy;
        bool bCrystalSpikes;
        bool breflect;
        uint8 uiCrystalSpikesCount;
        float fBaseX;
        float fBaseY;
        float fBaseZ;
        float fBaseO;
        float fSpikeXY[4][2];

        uint32 uiCrystalSpikesTimer;
        uint32 uiCrystalSpikesTimer2;
        uint32 uiTrampleTimer;
        uint32 uiFrenzyTimer;
        uint32 uiSpellReflectionTimer;
        uint32 uiReflect;
        uint32 uiSummonCrystallineTanglerTimer;

        uint8 uiReflectCount;

        void Reset()
        {
            uiCrystalSpikesTimer = 12*IN_MILLISECONDS;
            uiTrampleTimer = 10*IN_MILLISECONDS;
            uiSpellReflectionTimer = 30*IN_MILLISECONDS;
            uiSummonCrystallineTanglerTimer = 10*IN_MILLISECONDS;
            bFrenzy = false;
            bCrystalSpikes = false;
            uiReflect=14500;
            uiReflectCount=4;

            if (pInstance)
                pInstance->SetData(DATA_ORMOROK_EVENT, NOT_STARTED);
        }

        void EnterCombat(Unit* /*who*/)
        {
            DoScriptText(SAY_AGGRO, me);

            if (pInstance)
                pInstance->SetData(DATA_ORMOROK_EVENT, IN_PROGRESS);
        }

        void JustDied(Unit* /*killer*/)
        {
            DoScriptText(SAY_DEATH, me);

            if (pInstance)
                pInstance->SetData(DATA_ORMOROK_EVENT, DONE);
        }

        void KilledUnit(Unit* /*victim*/)
        {
            DoScriptText(SAY_KILL, me);
        }

        void SpellHit(Unit* /*pCaster*/, const SpellInfo* pSpell)
        {
            if (me->HasAura(SPELL_SPELL_REFLECTION)) //&& (!pSpell->AttributesEx2 & SPELL_ATTR_EX2_CANT_REFLECTED))
                uiReflectCount--;

            if (uiReflectCount==0)
                me->RemoveAura(SPELL_SPELL_REFLECTION,0,0,AURA_REMOVE_BY_EXPIRE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (bCrystalSpikes)
            {
                if (uiCrystalSpikesTimer2 <= diff)
                {
                    fSpikeXY[0][0] = fBaseX+(SPIKE_DISTANCE*uiCrystalSpikesCount*cos(fBaseO));
                    fSpikeXY[0][1] = fBaseY+(SPIKE_DISTANCE*uiCrystalSpikesCount*sin(fBaseO));
                    fSpikeXY[1][0] = fBaseX-(SPIKE_DISTANCE*uiCrystalSpikesCount*cos(fBaseO));
                    fSpikeXY[1][1] = fBaseY-(SPIKE_DISTANCE*uiCrystalSpikesCount*sin(fBaseO));
                    fSpikeXY[2][0] = fBaseX+(SPIKE_DISTANCE*uiCrystalSpikesCount*cos(fBaseO-(M_PI/2)));
                    fSpikeXY[2][1] = fBaseY+(SPIKE_DISTANCE*uiCrystalSpikesCount*sin(fBaseO-(M_PI/2)));
                    fSpikeXY[3][0] = fBaseX-(SPIKE_DISTANCE*uiCrystalSpikesCount*cos(fBaseO-(M_PI/2)));
                    fSpikeXY[3][1] = fBaseY-(SPIKE_DISTANCE*uiCrystalSpikesCount*sin(fBaseO-(M_PI/2)));
                    for (uint8 i = 0; i < 4; ++i)
                        me->SummonCreature(MOB_CRYSTAL_SPIKE, fSpikeXY[i][0], fSpikeXY[i][1], fBaseZ, 0, TEMPSUMMON_TIMED_DESPAWN, 7*IN_MILLISECONDS);
                    if (++uiCrystalSpikesCount >= 15)
                        bCrystalSpikes = false;
                    uiCrystalSpikesTimer2 = 200;
                } else uiCrystalSpikesTimer2 -= diff;
            }

            if (!bFrenzy && HealthBelowPct(25))
            {
                DoCast(me, SPELL_FRENZY);
                bFrenzy = true;
            }

            if (uiTrampleTimer <= diff)
            {
                DoCast(me, DUNGEON_MODE(SPELL_TRAMPLE,H_SPELL_TRAMPLE));
                uiTrampleTimer = 10*IN_MILLISECONDS;
            } else uiTrampleTimer -= diff;

            if (uiSpellReflectionTimer <= diff)
            {
                DoScriptText(SAY_REFLECT, me);
                DoCast(me, SPELL_SPELL_REFLECTION);
                uiSpellReflectionTimer = urand(25*IN_MILLISECONDS, 30*IN_MILLISECONDS);
                uiReflectCount=4;
            } else uiSpellReflectionTimer -= diff;

            if (breflect)
                if (uiReflect<=diff)
                {
                    me->RemoveAura(SPELL_SPELL_REFLECTION,0,0,AURA_REMOVE_BY_EXPIRE);
                    uiReflect=14500;
                } else uiReflect-=diff;

            if (uiCrystalSpikesTimer <= diff)
            {
                DoScriptText(SAY_CRYSTAL_SPIKES, me);
                bCrystalSpikes = true;
                uiCrystalSpikesCount = 1;
                uiCrystalSpikesTimer2 = 0;
                fBaseX = me->GetPositionX();
                fBaseY = me->GetPositionY();
                fBaseZ = me->GetPositionZ();
                fBaseO = me->GetOrientation();
                uiCrystalSpikesTimer = urand(15*IN_MILLISECONDS, 20*IN_MILLISECONDS);
            } else uiCrystalSpikesTimer -= diff;

            if (IsHeroic() && (uiSummonCrystallineTanglerTimer <= diff))
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                {
                    if (Creature *pTangler = me->SummonCreature(MOB_CRYSTALLINE_TANGLER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 1000))
                    {
                        pTangler->CastSpell(pTangler, SPELL_TANGLE, true);
                        pTangler->AI()->AttackStart(target);
                        pTangler->getThreatManager().addThreat(target, 1.0f);
                    }
                }
                uiSummonCrystallineTanglerTimer = urand(5*IN_MILLISECONDS, 15*IN_MILLISECONDS);
            } else uiSummonCrystallineTanglerTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

};

class mob_crystal_spike : public CreatureScript
{
public:
    mob_crystal_spike() : CreatureScript("mob_crystal_spike") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new mob_crystal_spikeAI (creature);
    }

    struct mob_crystal_spikeAI : public Scripted_NoMovementAI
    {
        mob_crystal_spikeAI(Creature* c) : Scripted_NoMovementAI(c)
        {
        }

        uint32 SpellCrystalSpikeDamageTimer;
        uint32 SpellCrystalSpikePrevisualTimer;

        void Reset()
        {
            SpellCrystalSpikeDamageTimer = 3700;
            SpellCrystalSpikePrevisualTimer = 1*IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff)
        {
            if (SpellCrystalSpikePrevisualTimer <= diff)
            {
                DoCast(me, SPELL_CRYSTAL_SPIKE_PREVISUAL);
                SpellCrystalSpikePrevisualTimer = 10*IN_MILLISECONDS;
            } else SpellCrystalSpikePrevisualTimer -= diff;

            if (SpellCrystalSpikeDamageTimer <= diff)
            {
            DoCast(me, DUNGEON_MODE(SPELL_CRYSTALL_SPIKE_DAMAGE,H_SPELL_CRYSTALL_SPIKE_DAMAGE));
                SpellCrystalSpikeDamageTimer = 10*IN_MILLISECONDS;
            } else SpellCrystalSpikeDamageTimer -= diff;
        }
    };

};

void AddSC_boss_ormorok()
{
    new boss_ormorok();
    new mob_crystal_spike();
}

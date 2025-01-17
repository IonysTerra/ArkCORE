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

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "SpellMgr.h"
#include "TicketMgr.h"
#include "MapManager.h"
#include "CreatureEventAIMgr.h"
#include "DisableMgr.h"
#include "LFGMgr.h"
#include "AuctionHouseMgr.h"
#include "CreatureTextMgr.h"
#include "SmartAI.h"
#include "SkillDiscovery.h"
#include "SkillExtraItems.h"
#include "Chat.h"

class reload_commandscript : public CommandScript
{
public:
    reload_commandscript() : CommandScript("reload_commandscript") { }

    ChatCommand* GetCommands() const
    {
        static ChatCommand reloadAllCommandTable[] =
        {
            { "achievement", SEC_ADMINISTRATOR,  true,  &HandleReloadAllAchievementCommand, "", NULL },
            { "area",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllAreaCommand,       "", NULL },
            { "eventai",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllEventAICommand,    "", NULL },
            { "gossips",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllGossipsCommand,    "", NULL },
            { "item",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllItemCommand,       "", NULL },
            { "locales",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllLocalesCommand,    "", NULL },
            { "loot",       SEC_ADMINISTRATOR,  true,  &HandleReloadAllLootCommand,       "", NULL },
            { "npc",        SEC_ADMINISTRATOR,  true,  &HandleReloadAllNpcCommand,        "", NULL },
            { "quest",      SEC_ADMINISTRATOR,  true,  &HandleReloadAllQuestCommand,      "", NULL },
            { "scripts",    SEC_ADMINISTRATOR,  true,  &HandleReloadAllScriptsCommand,    "", NULL },
            { "spell",      SEC_ADMINISTRATOR,  true,  &HandleReloadAllSpellCommand,      "", NULL },
            { "",           SEC_ADMINISTRATOR,  true,  &HandleReloadAllCommand,           "", NULL },
            { NULL,         0,                  false, NULL,                              "", NULL }
        };
        static ChatCommand reloadCommandTable[] =
        {
            { "auctions",                     SEC_ADMINISTRATOR, true,  &HandleReloadAuctionsCommand,                   "", NULL },
            { "access_requirement",           SEC_ADMINISTRATOR, true,  &HandleReloadAccessRequirementCommand,          "", NULL },
            { "achievement_criteria_data",    SEC_ADMINISTRATOR, true,  &HandleReloadAchievementCriteriaDataCommand,    "", NULL },
            { "achievement_reward",           SEC_ADMINISTRATOR, true,  &HandleReloadAchievementRewardCommand,          "", NULL },
            { "all",                          SEC_ADMINISTRATOR, true,  NULL,                          "", reloadAllCommandTable },
            { "areatrigger_involvedrelation", SEC_ADMINISTRATOR, true,  &HandleReloadQuestAreaTriggersCommand,          "", NULL },
            { "areatrigger_tavern",           SEC_ADMINISTRATOR, true,  &HandleReloadAreaTriggerTavernCommand,          "", NULL },
            { "areatrigger_teleport",         SEC_ADMINISTRATOR, true,  &HandleReloadAreaTriggerTeleportCommand,        "", NULL },
            { "autobroadcast",                SEC_ADMINISTRATOR, true,  &HandleReloadAutobroadcastCommand,              "", NULL },
            { "command",                      SEC_ADMINISTRATOR, true,  &HandleReloadCommandCommand,                    "", NULL },
            { "conditions",                   SEC_ADMINISTRATOR, true,  &HandleReloadConditions,                        "", NULL },
            { "config",                       SEC_ADMINISTRATOR, true,  &HandleReloadConfigCommand,                     "", NULL },
            { "creature_text",                SEC_ADMINISTRATOR, true,  &HandleReloadCreatureText,                      "", NULL },
            { "creature_ai_scripts",          SEC_ADMINISTRATOR, true,  &HandleReloadEventAIScriptsCommand,             "", NULL },
            { "creature_ai_summons",          SEC_ADMINISTRATOR, true,  &HandleReloadEventAISummonsCommand,             "", NULL },
            { "creature_ai_texts",            SEC_ADMINISTRATOR, true,  &HandleReloadEventAITextsCommand,               "", NULL },
            { "creature_involvedrelation",    SEC_ADMINISTRATOR, true,  &HandleReloadCreatureQuestInvRelationsCommand,  "", NULL },
            { "creature_linked_respawn",      SEC_GAMEMASTER,    true,  &HandleReloadLinkedRespawnCommand,              "", NULL },
            { "creature_loot_template",       SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesCreatureCommand,      "", NULL },
            { "creature_onkill_reputation",   SEC_ADMINISTRATOR, true,  &HandleReloadOnKillReputationCommand,           "", NULL },
            { "creature_questrelation",       SEC_ADMINISTRATOR, true,  &HandleReloadCreatureQuestRelationsCommand,     "", NULL },
            { "creature_template",            SEC_ADMINISTRATOR, true,  &HandleReloadCreatureTemplateCommand,           "", NULL },
            { "disables",                     SEC_ADMINISTRATOR, true,  &HandleReloadDisablesCommand,                   "", NULL },
            { "disenchant_loot_template",     SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesDisenchantCommand,    "", NULL },
            { "event_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadEventScriptsCommand,               "", NULL },
            { "fishing_loot_template",        SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesFishingCommand,       "", NULL },
            { "game_graveyard_zone",          SEC_ADMINISTRATOR, true,  &HandleReloadGameGraveyardZoneCommand,          "", NULL },
            { "game_tele",                    SEC_ADMINISTRATOR, true,  &HandleReloadGameTeleCommand,                   "", NULL },
            { "gameobject_involvedrelation",  SEC_ADMINISTRATOR, true,  &HandleReloadGOQuestInvRelationsCommand,        "", NULL },
            { "gameobject_loot_template",     SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesGameobjectCommand,    "", NULL },
            { "gameobject_questrelation",     SEC_ADMINISTRATOR, true,  &HandleReloadGOQuestRelationsCommand,           "", NULL },
            { "gameobject_scripts",           SEC_ADMINISTRATOR, true,  &HandleReloadGameObjectScriptsCommand,          "", NULL },
            { "gm_tickets",                   SEC_ADMINISTRATOR, true,  &HandleReloadGMTicketsCommand,                  "", NULL },
            { "gossip_menu",                  SEC_ADMINISTRATOR, true,  &HandleReloadGossipMenuCommand,                 "", NULL },
            { "gossip_menu_option",           SEC_ADMINISTRATOR, true,  &HandleReloadGossipMenuOptionCommand,           "", NULL },
            { "gossip_scripts",               SEC_ADMINISTRATOR, true,  &HandleReloadGossipScriptsCommand,              "", NULL },
            { "item_enchantment_template",    SEC_ADMINISTRATOR, true,  &HandleReloadItemEnchantementsCommand,          "", NULL },
            { "item_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesItemCommand,          "", NULL },
            { "lfg_dungeon_rewards",          SEC_ADMINISTRATOR, true,  &HandleReloadLfgRewardsCommand,                 "", NULL },
            { "locales_achievement_reward",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesAchievementRewardCommand,   "", NULL },
            { "locales_creature",             SEC_ADMINISTRATOR, true,  &HandleReloadLocalesCreatureCommand,            "", NULL },
            { "locales_gameobject",           SEC_ADMINISTRATOR, true,  &HandleReloadLocalesGameobjectCommand,          "", NULL },
            { "locales_gossip_menu_option",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesGossipMenuOptionCommand,    "", NULL },
            { "locales_item",                 SEC_ADMINISTRATOR, true,  &HandleReloadLocalesItemCommand,                "", NULL },
            { "locales_npc_text",             SEC_ADMINISTRATOR, true,  &HandleReloadLocalesNpcTextCommand,             "", NULL },
            { "locales_page_text",            SEC_ADMINISTRATOR, true,  &HandleReloadLocalesPageTextCommand,            "", NULL },
            { "locales_points_of_interest",   SEC_ADMINISTRATOR, true,  &HandleReloadLocalesPointsOfInterestCommand,    "", NULL },
            { "locales_quest",                SEC_ADMINISTRATOR, true,  &HandleReloadLocalesQuestCommand,               "", NULL },
            { "mail_level_reward",            SEC_ADMINISTRATOR, true,  &HandleReloadMailLevelRewardCommand,            "", NULL },
            { "mail_loot_template",           SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesMailCommand,          "", NULL },
            { "milling_loot_template",        SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesMillingCommand,       "", NULL },
            { "npc_spellclick_spells",        SEC_ADMINISTRATOR, true,  &HandleReloadSpellClickSpellsCommand,           "", NULL},
            { "npc_trainer",                  SEC_ADMINISTRATOR, true,  &HandleReloadNpcTrainerCommand,                 "", NULL },
            { "npc_vendor",                   SEC_ADMINISTRATOR, true,  &HandleReloadNpcVendorCommand,                  "", NULL },
            { "page_text",                    SEC_ADMINISTRATOR, true,  &HandleReloadPageTextsCommand,                  "", NULL },
            { "pickpocketing_loot_template",  SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesPickpocketingCommand, "", NULL},
            { "points_of_interest",           SEC_ADMINISTRATOR, true,  &HandleReloadPointsOfInterestCommand,           "", NULL },
            { "prospecting_loot_template",    SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesProspectingCommand,   "", NULL },
            { "quest_end_scripts",            SEC_ADMINISTRATOR, true,  &HandleReloadQuestEndScriptsCommand,            "", NULL },
            { "quest_poi",                    SEC_ADMINISTRATOR, true,  &HandleReloadQuestPOICommand,                   "", NULL },
            { "quest_start_scripts",          SEC_ADMINISTRATOR, true,  &HandleReloadQuestStartScriptsCommand,          "", NULL },
            { "quest_template",               SEC_ADMINISTRATOR, true,  &HandleReloadQuestTemplateCommand,              "", NULL },
            { "reference_loot_template",      SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesReferenceCommand,     "", NULL },
            { "reserved_name",                SEC_ADMINISTRATOR, true,  &HandleReloadReservedNameCommand,               "", NULL },
            { "reputation_reward_rate",       SEC_ADMINISTRATOR, true,  &HandleReloadReputationRewardRateCommand,       "", NULL },
            { "reputation_spillover_template", SEC_ADMINISTRATOR, true,  &HandleReloadReputationRewardRateCommand,       "", NULL },
            { "skill_discovery_template",     SEC_ADMINISTRATOR, true,  &HandleReloadSkillDiscoveryTemplateCommand,     "", NULL },
            { "skill_extra_item_template",    SEC_ADMINISTRATOR, true,  &HandleReloadSkillExtraItemTemplateCommand,     "", NULL },
            { "skill_fishing_base_level",     SEC_ADMINISTRATOR, true,  &HandleReloadSkillFishingBaseLevelCommand,      "", NULL },
            { "skinning_loot_template",       SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesSkinningCommand,      "", NULL },
            { "smart_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadSmartScripts,                      "", NULL },
            { "spell_required",               SEC_ADMINISTRATOR, true,  &HandleReloadSpellRequiredCommand,              "", NULL },
            { "spell_area",                   SEC_ADMINISTRATOR, true,  &HandleReloadSpellAreaCommand,                  "", NULL },
            { "spell_map",                    SEC_ADMINISTRATOR, true,  &HandleReloadSpellMapCommand,                  "", NULL },
            { "spell_bonus_data",             SEC_ADMINISTRATOR, true,  &HandleReloadSpellBonusesCommand,               "", NULL },
            { "spell_group",                  SEC_ADMINISTRATOR, true,  &HandleReloadSpellGroupsCommand,                "", NULL },
            { "spell_learn_spell",            SEC_ADMINISTRATOR, true,  &HandleReloadSpellLearnSpellCommand,            "", NULL },
            { "spell_loot_template",          SEC_ADMINISTRATOR, true,  &HandleReloadLootTemplatesSpellCommand,         "", NULL },
            { "spell_linked_spell",           SEC_ADMINISTRATOR, true,  &HandleReloadSpellLinkedSpellCommand,           "", NULL },
            { "spell_pet_auras",              SEC_ADMINISTRATOR, true,  &HandleReloadSpellPetAurasCommand,              "", NULL },
            { "spell_proc_event",             SEC_ADMINISTRATOR, true,  &HandleReloadSpellProcEventCommand,             "", NULL },
            { "spell_proc",                   SEC_ADMINISTRATOR, true,  &HandleReloadSpellProcsCommand,             "", NULL },
            { "spell_scripts",                SEC_ADMINISTRATOR, true,  &HandleReloadSpellScriptsCommand,               "", NULL },
            { "spell_target_position",        SEC_ADMINISTRATOR, true,  &HandleReloadSpellTargetPositionCommand,        "", NULL },
            { "spell_threats",                SEC_ADMINISTRATOR, true,  &HandleReloadSpellThreatsCommand,               "", NULL },
            { "spell_group_stack_rules",      SEC_ADMINISTRATOR, true,  &HandleReloadSpellGroupStackRulesCommand,       "", NULL },
            { "arkcore_string",              SEC_ADMINISTRATOR, true,  &HandleReloadArkcoreStringCommand,              "", NULL },
            { "waypoint_scripts",             SEC_ADMINISTRATOR, true,  &HandleReloadWpScriptsCommand,                  "", NULL },
            { "vehicle_accessory",            SEC_ADMINISTRATOR, true,  &HandleReloadVehicleAccessoryCommand,           "", NULL },
            { "vehicle_template_accessory",   SEC_ADMINISTRATOR, true,  &HandleReloadVehicleTemplateAccessoryCommand,   "", NULL },
            { "creature_onkill_currency",     SEC_ADMINISTRATOR, true,  &HandleReloadCreatureOnKillCurrencyCommand,     "", NULL },
            { NULL,                           0,                 false, NULL,                                           "", NULL }
        };
        static ChatCommand commandTable[] =
        {
            { "reload",         SEC_ADMINISTRATOR,  true,  NULL,                 "", reloadCommandTable },
            { "reload all",     SEC_ADMINISTRATOR,  true,  NULL,                 "", reloadAllCommandTable },
            { NULL,             0,                  false, NULL,                               "", NULL }
        };
        return commandTable;
    }

    //reload commands
    static bool HandleReloadGMTicketsCommand(ChatHandler* /*handler*/, const char* /*args*/)
    {
        sTicketMgr->LoadTickets();
        return true;
    }

    static bool HandleReloadAllCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadSkillFishingBaseLevelCommand(handler, "");

        HandleReloadAllAchievementCommand(handler, "");
        HandleReloadAllAreaCommand(handler, "");
        HandleReloadAllEventAICommand(handler, "");
        HandleReloadAllLootCommand(handler, "");
        HandleReloadAllNpcCommand(handler, "");
        HandleReloadAllQuestCommand(handler, "");
        HandleReloadAllSpellCommand(handler, "");
        HandleReloadAllItemCommand(handler, "");
        HandleReloadAllGossipsCommand(handler, "");
        HandleReloadAllLocalesCommand(handler, "");

        HandleReloadAccessRequirementCommand(handler, "");
        HandleReloadMailLevelRewardCommand(handler, "");
        HandleReloadCommandCommand(handler, "");
        HandleReloadReservedNameCommand(handler, "");
        HandleReloadArkcoreStringCommand(handler, "");
        HandleReloadGameTeleCommand(handler, "");

        HandleReloadVehicleAccessoryCommand(handler, "");
        HandleReloadVehicleTemplateAccessoryCommand(handler, "");

        HandleReloadAutobroadcastCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllAchievementCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadAchievementCriteriaDataCommand(handler, "");
        HandleReloadAchievementRewardCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllAreaCommand(ChatHandler* handler, const char* /*args*/)
    {
        //HandleReloadQuestAreaTriggersCommand(handler, ""); -- reloaded in HandleReloadAllQuestCommand
        HandleReloadAreaTriggerTeleportCommand(handler, "");
        HandleReloadAreaTriggerTavernCommand(handler, "");
        HandleReloadGameGraveyardZoneCommand(handler, "");
        return true;
    }

    static bool HandleReloadAllLootCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables...");
        LoadLootTables();
        handler->SendGlobalGMSysMessage("DB tables `*_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadAllNpcCommand(ChatHandler* handler, const char* args)
    {
        if (*args != 'a')                                          // will be reloaded from all_gossips
        HandleReloadNpcTrainerCommand(handler, "a");
        HandleReloadNpcVendorCommand(handler, "a");
        HandleReloadPointsOfInterestCommand(handler, "a");
        HandleReloadSpellClickSpellsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllQuestCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadQuestAreaTriggersCommand(handler, "a");
        HandleReloadQuestPOICommand(handler, "a");
        HandleReloadQuestTemplateCommand(handler, "a");

        sLog->outString("Re-Loading Quests Relations...");
        sObjectMgr->LoadQuestRelations();
        handler->SendGlobalGMSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
        return true;
    }

    static bool HandleReloadAllScriptsCommand(ChatHandler* handler, const char* /*args*/)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->PSendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        sLog->outString("Re-Loading Scripts...");
        HandleReloadGameObjectScriptsCommand(handler, "a");
        HandleReloadGossipScriptsCommand(handler, "a");
        HandleReloadEventScriptsCommand(handler, "a");
        HandleReloadQuestEndScriptsCommand(handler, "a");
        HandleReloadQuestStartScriptsCommand(handler, "a");
        HandleReloadSpellScriptsCommand(handler, "a");
        handler->SendGlobalGMSysMessage("DB tables `*_scripts` reloaded.");
        HandleReloadDbScriptStringCommand(handler, "a");
        HandleReloadWpScriptsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllEventAICommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadEventAITextsCommand(handler, "a");
        HandleReloadEventAISummonsCommand(handler, "a");
        HandleReloadEventAIScriptsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadSkillDiscoveryTemplateCommand(handler, "a");
        HandleReloadSkillExtraItemTemplateCommand(handler, "a");
        HandleReloadSpellRequiredCommand(handler, "a");
        HandleReloadSpellAreaCommand(handler, "a");
        HandleReloadSpellGroupsCommand(handler, "a");
        HandleReloadSpellLearnSpellCommand(handler, "a");
        HandleReloadSpellLinkedSpellCommand(handler, "a");
        HandleReloadSpellProcEventCommand(handler, "a");
        HandleReloadSpellProcsCommand(handler, "a");
        HandleReloadSpellBonusesCommand(handler, "a");
        HandleReloadSpellTargetPositionCommand(handler, "a");
        HandleReloadSpellThreatsCommand(handler, "a");
        HandleReloadSpellGroupStackRulesCommand(handler, "a");
        HandleReloadSpellPetAurasCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllGossipsCommand(ChatHandler* handler, const char* args)
    {
        HandleReloadGossipMenuCommand(handler, "a");
        HandleReloadGossipMenuOptionCommand(handler, "a");
        if (*args != 'a')                                          // already reload from all_scripts
        HandleReloadGossipScriptsCommand(handler, "a");
        HandleReloadPointsOfInterestCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllItemCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadPageTextsCommand(handler, "a");
        HandleReloadItemEnchantementsCommand(handler, "a");
        return true;
    }

    static bool HandleReloadAllLocalesCommand(ChatHandler* handler, const char* /*args*/)
    {
        HandleReloadLocalesAchievementRewardCommand(handler, "a");
        HandleReloadLocalesCreatureCommand(handler, "a");
        HandleReloadLocalesGameobjectCommand(handler, "a");
        HandleReloadLocalesGossipMenuOptionCommand(handler, "a");
        HandleReloadLocalesItemCommand(handler, "a");
        HandleReloadLocalesNpcTextCommand(handler, "a");
        HandleReloadLocalesPageTextCommand(handler, "a");
        HandleReloadLocalesPointsOfInterestCommand(handler, "a");
        HandleReloadLocalesQuestCommand(handler, "a");
        return true;
    }

    static bool HandleReloadConfigCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading config settings...");
        sWorld->LoadConfigSettings(true);
        sMapMgr->InitializeVisibilityDistanceInfo();
        handler->SendGlobalGMSysMessage("World config settings reloaded.");
        return true;
    }

    static bool HandleReloadAccessRequirementCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Access Requirement definitions...");
        sObjectMgr->LoadAccessRequirements();
        handler->SendGlobalGMSysMessage("DB table `access_requirement` reloaded.");
        return true;
    }

    static bool HandleReloadAchievementCriteriaDataCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Additional Achievement Criteria Data...");
        sAchievementMgr->LoadAchievementCriteriaData();
        handler->SendGlobalGMSysMessage("DB table `achievement_criteria_data` reloaded.");
        return true;
    }

    static bool HandleReloadAchievementRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Achievement Reward Data...");
        sAchievementMgr->LoadRewards();
        handler->SendGlobalGMSysMessage("DB table `achievement_reward` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTavernCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Tavern Area Triggers...");
        sObjectMgr->LoadTavernAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_tavern` reloaded.");
        return true;
    }

    static bool HandleReloadAreaTriggerTeleportCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading AreaTrigger teleport definitions...");
        sObjectMgr->LoadAreaTriggerTeleports();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_teleport` reloaded.");
        return true;
    }

    static bool HandleReloadAutobroadcastCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Autobroadcasts...");
        sWorld->LoadAutobroadcasts();
        handler->SendGlobalGMSysMessage("DB table `autobroadcast` reloaded.");
        return true;
    }

    static bool HandleReloadCommandCommand(ChatHandler* handler, const char* /*args*/)
    {
        handler->SetLoadCommandTable(true);
        handler->SendGlobalGMSysMessage("DB table `command` will be reloaded at next chat command use.");
        return true;
    }

    static bool HandleReloadOnKillReputationCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading creature award reputation definitions...");
        sObjectMgr->LoadReputationOnKill();
        handler->SendGlobalGMSysMessage("DB table `creature_onkill_reputation` reloaded.");
        return true;
    }

    static bool HandleReloadCreatureTemplateCommand(ChatHandler* handler, const char* args)
    {
        if (!*args)
            return false;

        uint32 entry = (uint32) atoi((char*)args);
        QueryResult result = WorldDatabase.PQuery("SELECT difficulty_entry_1, difficulty_entry_2, difficulty_entry_3, KillCredit1, KillCredit2, modelid1, modelid2, modelid3, modelid4, name, subname, IconName, gossip_menu_id, minlevel, maxlevel, exp, faction_A, faction_H, npcflag, speed_walk, speed_run, scale, rank, mindmg, maxdmg, dmgschool, attackpower, dmg_multiplier, baseattacktime, rangeattacktime, unit_class, unit_flags, dynamicflags, family, trainer_type, trainer_spell, trainer_class, trainer_race, minrangedmg, maxrangedmg, rangedattackpower, type, type_flags, lootid, pickpocketloot, skinloot, resistance1, resistance2, resistance3, resistance4, resistance5, resistance6, spell1, spell2, spell3, spell4, spell5, spell6, spell7, spell8, PetSpellDataId, VehicleId, mingold, maxgold, AIName, MovementType, InhabitType, Health_mod, Mana_mod, Armor_mod, RacialLeader, questItem1, questItem2, questItem3, questItem4, questItem5, questItem6, movementId, RegenHealth, equipment_id, mechanic_immune_mask, flags_extra, ScriptName FROM creature_template WHERE entry = %u", entry);
        if (!result)
        {
            handler->PSendSysMessage(LANG_COMMAND_CREATURETEMPLATE_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(entry);
        if (!cInfo)
        {
            handler->PSendSysMessage(LANG_COMMAND_CREATURESTORAGE_NOTFOUND, entry);
            handler->SetSentErrorMessage(true);
            return false;
        }

        sLog->outString("Reloading creature template entry %u", entry);

        Field* fields = result->Fetch();

        const_cast<CreatureTemplate*>(cInfo)->DifficultyEntry[0] = fields[0].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->DifficultyEntry[1] = fields[1].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->DifficultyEntry[2] = fields[2].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->KillCredit[0] = fields[3].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->KillCredit[1] = fields[4].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->Modelid1 = fields[5].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->Modelid2 = fields[6].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->Modelid3 = fields[7].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->Modelid4 = fields[8].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->Name = fields[9].GetString();
        const_cast<CreatureTemplate*>(cInfo)->SubName = fields[10].GetString();
        const_cast<CreatureTemplate*>(cInfo)->IconName = fields[11].GetString();
        const_cast<CreatureTemplate*>(cInfo)->GossipMenuId = fields[12].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->minlevel = fields[13].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->maxlevel = fields[14].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->expansion = fields[15].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->faction_A = fields[16].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->faction_H = fields[17].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->npcflag = fields[18].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->speed_walk = fields[19].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->speed_run = fields[20].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->scale = fields[21].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->rank = fields[22].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->mindmg = fields[23].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->maxdmg = fields[24].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->dmgschool = fields[25].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->attackpower = fields[26].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->dmg_multiplier = fields[27].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->baseattacktime = fields[28].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->rangeattacktime = fields[29].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->unit_class = fields[30].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->unit_flags = fields[31].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->dynamicflags = fields[32].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->family = fields[33].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->trainer_type = fields[34].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->trainer_spell = fields[35].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->trainer_class = fields[36].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->trainer_race = fields[37].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->minrangedmg = fields[38].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->maxrangedmg = fields[39].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->rangedattackpower = fields[40].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->type = fields[41].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->type_flags = fields[42].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->lootid = fields[43].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->pickpocketLootId = fields[44].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->SkinLootId = fields[45].GetUInt32();

        for (uint8 i = SPELL_SCHOOL_HOLY; i < MAX_SPELL_SCHOOL; ++i)
        {
            const_cast<CreatureTemplate*>(cInfo)->resistance[i] = fields[46 + i -1].GetUInt32();
        }

        const_cast<CreatureTemplate*>(cInfo)->spells[0] = fields[52].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[1] = fields[53].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[2] = fields[54].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[3] = fields[55].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[4] = fields[56].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[5] = fields[57].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[6] = fields[58].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->spells[7] = fields[59].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->PetSpellDataId = fields[60].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->VehicleId = fields[61].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->mingold = fields[62].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->maxgold = fields[63].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->AIName = fields[64].GetString();
        const_cast<CreatureTemplate*>(cInfo)->MovementType = fields[65].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->InhabitType = fields[66].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->ModHealth = fields[67].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->ModMana = fields[68].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->ModArmor = fields[69].GetFloat();
        const_cast<CreatureTemplate*>(cInfo)->RacialLeader = fields[70].GetBool();
        const_cast<CreatureTemplate*>(cInfo)->questItems[0] = fields[71].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->questItems[1] = fields[72].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->questItems[2] = fields[73].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->questItems[3] = fields[74].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->questItems[4] = fields[75].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->questItems[5] = fields[76].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->movementId = fields[77].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->RegenHealth = fields[78].GetBool();
        const_cast<CreatureTemplate*>(cInfo)->equipmentId = fields[79].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->MechanicImmuneMask = fields[80].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->flags_extra = fields[81].GetUInt32();
        const_cast<CreatureTemplate*>(cInfo)->ScriptID = sObjectMgr->GetScriptId(fields[82].GetCString());

        sObjectMgr->CheckCreatureTemplate(cInfo);

        handler->SendGlobalGMSysMessage("Creature template reloaded.");
        return true;
    }

    static bool HandleReloadCreatureQuestRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading Quests Relations... (`creature_questrelation`)");
        sObjectMgr->LoadCreatureQuestRelations();
        handler->SendGlobalGMSysMessage("DB table `creature_questrelation` (creature quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadLinkedRespawnCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading Linked Respawns... (`creature_linked_respawn`)");
        sObjectMgr->LoadLinkedRespawn();
        handler->SendGlobalGMSysMessage("DB table `creature_linked_respawn` (creature linked respawns) reloaded.");
        return true;
    }

    static bool HandleReloadCreatureQuestInvRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading Quests Relations... (`creature_involvedrelation`)");
        sObjectMgr->LoadCreatureInvolvedRelations();
        handler->SendGlobalGMSysMessage("DB table `creature_involvedrelation` (creature quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadGossipMenuCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `gossip_menu` Table!");
        sObjectMgr->LoadGossipMenu();
        handler->SendGlobalGMSysMessage("DB table `gossip_menu` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadGossipMenuOptionCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `gossip_menu_option` Table!");
        sObjectMgr->LoadGossipMenuItems();
        handler->SendGlobalGMSysMessage("DB table `gossip_menu_option` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadGOQuestRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading Quests Relations... (`gameobject_questrelation`)");
        sObjectMgr->LoadGameobjectQuestRelations();
        handler->SendGlobalGMSysMessage("DB table `gameobject_questrelation` (gameobject quest givers) reloaded.");
        return true;
    }

    static bool HandleReloadGOQuestInvRelationsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading Quests Relations... (`gameobject_involvedrelation`)");
        sObjectMgr->LoadGameobjectInvolvedRelations();
        handler->SendGlobalGMSysMessage("DB table `gameobject_involvedrelation` (gameobject quest takers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestAreaTriggersCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Quest Area Triggers...");
        sObjectMgr->LoadQuestAreaTriggers();
        handler->SendGlobalGMSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
        return true;
    }

    static bool HandleReloadQuestTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Quest Templates...");
        sObjectMgr->LoadQuests();
        handler->SendGlobalGMSysMessage("DB table `quest_template` (quest definitions) reloaded.");

        /// dependent also from `gameobject` but this table not reloaded anyway
        sLog->outString("Re-Loading GameObjects for quests...");
        sObjectMgr->LoadGameObjectForQuests();
        handler->SendGlobalGMSysMessage("Data GameObjects for quests reloaded.");
        return true;
    }

    static bool HandleReloadLootTemplatesCreatureCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`creature_loot_template`)");
        LoadLootTemplates_Creature();
        LootTemplates_Creature.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `creature_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesDisenchantCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`disenchant_loot_template`)");
        LoadLootTemplates_Disenchant();
        LootTemplates_Disenchant.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `disenchant_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesFishingCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`fishing_loot_template`)");
        LoadLootTemplates_Fishing();
        LootTemplates_Fishing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `fishing_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesGameobjectCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`gameobject_loot_template`)");
        LoadLootTemplates_Gameobject();
        LootTemplates_Gameobject.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `gameobject_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesItemCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`item_loot_template`)");
        LoadLootTemplates_Item();
        LootTemplates_Item.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `item_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesMillingCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`milling_loot_template`)");
        LoadLootTemplates_Milling();
        LootTemplates_Milling.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `milling_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesPickpocketingCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`pickpocketing_loot_template`)");
        LoadLootTemplates_Pickpocketing();
        LootTemplates_Pickpocketing.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `pickpocketing_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesProspectingCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`prospecting_loot_template`)");
        LoadLootTemplates_Prospecting();
        LootTemplates_Prospecting.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `prospecting_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesMailCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`mail_loot_template`)");
        LoadLootTemplates_Mail();
        LootTemplates_Mail.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `mail_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesReferenceCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`reference_loot_template`)");
        LoadLootTemplates_Reference();
        handler->SendGlobalGMSysMessage("DB table `reference_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesSkinningCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`skinning_loot_template`)");
        LoadLootTemplates_Skinning();
        LootTemplates_Skinning.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `skinning_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadLootTemplatesSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Loot Tables... (`spell_loot_template`)");
        LoadLootTemplates_Spell();
        LootTemplates_Spell.CheckLootRefs();
        handler->SendGlobalGMSysMessage("DB table `spell_loot_template` reloaded.");
        sConditionMgr->LoadConditions(true);
        return true;
    }

    static bool HandleReloadArkcoreStringCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading arkcore_string Table!");
        sObjectMgr->LoadArkcoreStrings();
        handler->SendGlobalGMSysMessage("DB table `arkcore_string` reloaded.");
        return true;
    }

    static bool HandleReloadNpcTrainerCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `npc_trainer` Table!");
        sObjectMgr->LoadTrainerSpell();
        handler->SendGlobalGMSysMessage("DB table `npc_trainer` reloaded.");
        return true;
    }

    static bool HandleReloadNpcVendorCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `npc_vendor` Table!");
        sObjectMgr->LoadVendors();
        handler->SendGlobalGMSysMessage("DB table `npc_vendor` reloaded.");
        return true;
    }

    static bool HandleReloadPointsOfInterestCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `points_of_interest` Table!");
        sObjectMgr->LoadPointsOfInterest();
        handler->SendGlobalGMSysMessage("DB table `points_of_interest` reloaded.");
        return true;
    }

    static bool HandleReloadQuestPOICommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString( "Re-Loading Quest POI ..." );
        sObjectMgr->LoadQuestPOI();
        handler->SendGlobalGMSysMessage("DB Table `quest_poi` and `quest_poi_points` reloaded.");
        return true;
    }

    static bool HandleReloadSpellClickSpellsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading `npc_spellclick_spells` Table!");
        sObjectMgr->LoadNPCSpellClickSpells();
        handler->SendGlobalGMSysMessage("DB table `npc_spellclick_spells` reloaded.");
        return true;
    }

    static bool HandleReloadReservedNameCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Loading ReservedNames... (`reserved_name`)");
        sObjectMgr->LoadReservedPlayersNames();
        handler->SendGlobalGMSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
        return true;
    }

    static bool HandleReloadReputationRewardRateCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString( "Re-Loading `reputation_reward_rate` Table!" );
        sObjectMgr->LoadReputationRewardRate();
        handler->SendGlobalSysMessage("DB table `reputation_reward_rate` reloaded.");
        return true;
    }

    static bool HandleReloadReputationSpilloverTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString( "Re-Loading `reputation_spillover_template` Table!" );
        sObjectMgr->LoadReputationSpilloverTemplate();
        handler->SendGlobalSysMessage("DB table `reputation_spillover_template` reloaded.");
        return true;
    }

    static bool HandleReloadSkillDiscoveryTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Skill Discovery Table...");
        LoadSkillDiscoveryTable();
        handler->SendGlobalGMSysMessage("DB table `skill_discovery_template` (recipes discovered at crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillExtraItemTemplateCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Skill Extra Item Table...");
        LoadSkillExtraItemTable();
        handler->SendGlobalGMSysMessage("DB table `skill_extra_item_template` (extra item creation when crafting) reloaded.");
        return true;
    }

    static bool HandleReloadSkillFishingBaseLevelCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Skill Fishing base level requirements...");
        sObjectMgr->LoadFishingBaseSkillLevel();
        handler->SendGlobalGMSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
        return true;
    }

    static bool HandleReloadSpellAreaCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading SpellArea Data...");
        sSpellMgr->LoadSpellAreas();
        handler->SendGlobalGMSysMessage("DB table `spell_area` (spell dependences from area/quest/auras state) reloaded.");
        return true;
    }

    static bool HandleReloadSpellMapCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading SpellMap Data...");
        sSpellMgr->LoadSpellMaps();
        handler->SendGlobalGMSysMessage("DB table `spell_map` (spell dependences from map/quest/auras state) reloaded.");
        return true;
    }

    static bool HandleReloadSpellRequiredCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Required Data... ");
        sSpellMgr->LoadSpellRequired();
        handler->SendGlobalGMSysMessage("DB table `spell_required` reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Groups...");
        sSpellMgr->LoadSpellGroups();
        handler->SendGlobalGMSysMessage("DB table `spell_group` (spell groups) reloaded.");
        return true;
    }

    static bool HandleReloadSpellLearnSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Learn Spells...");
        sSpellMgr->LoadSpellLearnSpells();
        handler->SendGlobalGMSysMessage("DB table `spell_learn_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellLinkedSpellCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Linked Spells...");
        sSpellMgr->LoadSpellLinked();
        handler->SendGlobalGMSysMessage("DB table `spell_linked_spell` reloaded.");
        return true;
    }

    static bool HandleReloadSpellProcEventCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Proc Event conditions...");
        sSpellMgr->LoadSpellProcEvents();
        handler->SendGlobalGMSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
        return true;
    }

    static bool HandleReloadSpellProcsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Proc conditions and data...");
        sSpellMgr->LoadSpellProcs();
        handler->SendGlobalGMSysMessage("DB table `spell_proc` (spell proc conditions and data) reloaded.");
        return true;
    }

    static bool HandleReloadSpellBonusesCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Bonus Data...");
        sSpellMgr->LoadSpellBonusess();
        handler->SendGlobalGMSysMessage("DB table `spell_bonus_data` (spell damage/healing coefficients) reloaded.");
        return true;
    }

    static bool HandleReloadSpellTargetPositionCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell target coordinates...");
        sSpellMgr->LoadSpellTargetPositions();
        handler->SendGlobalGMSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
        return true;
    }

    static bool HandleReloadSpellThreatsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Aggro Spells Definitions...");
        sSpellMgr->LoadSpellThreats();
        handler->SendGlobalGMSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellGroupStackRulesCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell Group Stack Rules...");
        sSpellMgr->LoadSpellGroupStackRules();
        handler->SendGlobalGMSysMessage("DB table `spell_group_stack_rules` (spell stacking definitions) reloaded.");
        return true;
    }

    static bool HandleReloadSpellPetAurasCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Spell pet auras...");
        sSpellMgr->LoadSpellPetAuras();
        handler->SendGlobalGMSysMessage("DB table `spell_pet_auras` reloaded.");
        return true;
    }

    static bool HandleReloadPageTextsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Page Texts...");
        sObjectMgr->LoadPageTexts();
        handler->SendGlobalGMSysMessage("DB table `page_texts` reloaded.");
        return true;
    }

    static bool HandleReloadItemEnchantementsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Item Random Enchantments Table...");
        LoadRandomEnchantmentsTable();
        handler->SendGlobalGMSysMessage("DB table `item_enchantment_template` reloaded.");
        return true;
    }

    static bool HandleReloadGossipScriptsCommand(ChatHandler* handler, const char* args)
    {

        if (sScriptMgr->IsScriptScheduled())

        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `gossip_scripts`...");

    		sObjectMgr->LoadGossipScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `gossip_scripts` reloaded.");

			return true;
    }

    static bool HandleReloadGameObjectScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `gameobject_scripts`...");

        sObjectMgr->LoadGameObjectScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `gameobject_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadEventScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `event_scripts`...");

        sObjectMgr->LoadEventScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `event_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadWpScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `waypoint_scripts`...");

        sObjectMgr->LoadWaypointScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `waypoint_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadEventAITextsCommand(ChatHandler* handler, const char* /*args*/)
    {

        sLog->outString("Re-Loading Texts from `creature_ai_texts`...");
        sEventAIMgr->LoadCreatureEventAI_Texts();
        handler->SendGlobalGMSysMessage("DB table `creature_ai_texts` reloaded.");
        return true;
    }

    static bool HandleReloadEventAISummonsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Summons from `creature_ai_summons`...");
        sEventAIMgr->LoadCreatureEventAI_Summons();
        handler->SendGlobalGMSysMessage("DB table `creature_ai_summons` reloaded.");
        return true;
    }

    static bool HandleReloadEventAIScriptsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Scripts from `creature_ai_scripts`...");
        sEventAIMgr->LoadCreatureEventAI_Scripts();
        handler->SendGlobalGMSysMessage("DB table `creature_ai_scripts` reloaded.");
        return true;
    }

    static bool HandleReloadQuestEndScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `quest_end_scripts`...");

        sObjectMgr->LoadQuestEndScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_end_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadQuestStartScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `quest_start_scripts`...");

        sObjectMgr->LoadQuestStartScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `quest_start_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadSpellScriptsCommand(ChatHandler* handler, const char* args)
    {
        if (sScriptMgr->IsScriptScheduled())
        {
            handler->SendSysMessage("DB scripts used currently, please attempt reload later.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (*args != 'a')
            sLog->outString("Re-Loading Scripts from `spell_scripts`...");

        sObjectMgr->LoadSpellScripts();

        if (*args != 'a')
            handler->SendGlobalGMSysMessage("DB table `spell_scripts` reloaded.");

        return true;
    }

    static bool HandleReloadDbScriptStringCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Script strings from `db_script_string`...");
        sObjectMgr->LoadDbScriptStrings();
        handler->SendGlobalGMSysMessage("DB table `db_script_string` reloaded.");
        return true;
    }

    static bool HandleReloadGameGraveyardZoneCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Graveyard-zone links...");

        sObjectMgr->LoadGraveyardZones();

        handler->SendGlobalGMSysMessage("DB table `game_graveyard_zone` reloaded.");

        return true;
    }

    static bool HandleReloadGameTeleCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Game Tele coordinates...");

        sObjectMgr->LoadGameTele();

        handler->SendGlobalGMSysMessage("DB table `game_tele` reloaded.");

        return true;
    }

    static bool HandleReloadDisablesCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading disables table...");
        DisableMgr::LoadDisables();
        sLog->outString("Checking quest disables...");
        DisableMgr::CheckQuestDisables();
        handler->SendGlobalGMSysMessage("DB table `disables` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesAchievementRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Achievement Reward Data...");
        sAchievementMgr->LoadRewardLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_achievement_reward` reloaded.");
        return true;
    }

    static bool HandleReloadLfgRewardsCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading lfg dungeon rewards...");
        sLFGMgr->LoadRewards();
        handler->SendGlobalGMSysMessage("DB table `lfg_dungeon_rewards` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesCreatureCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Creature ...");
        sObjectMgr->LoadCreatureLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_creature` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGameobjectCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Gameobject ... ");
        sObjectMgr->LoadGameObjectLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gameobject` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesGossipMenuOptionCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString( "Re-Loading Locales Gossip Menu Option ... ");
        sObjectMgr->LoadGossipMenuItemsLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_gossip_menu_option` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesItemCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Item ... ");
        sObjectMgr->LoadItemLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_item` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesNpcTextCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales NPC Text ... ");
        sObjectMgr->LoadNpcTextLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_npc_text` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesPageTextCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Page Text ... ");
        sObjectMgr->LoadPageTextLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_page_text` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesPointsOfInterestCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Points Of Interest ... ");
        sObjectMgr->LoadPointOfInterestLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_points_of_interest` reloaded.");
        return true;
    }

    static bool HandleReloadLocalesQuestCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Locales Quest ... ");
        sObjectMgr->LoadQuestLocales();
        handler->SendGlobalGMSysMessage("DB table `locales_quest` reloaded.");
        return true;
    }

    static bool HandleReloadMailLevelRewardCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Player level dependent mail rewards...");
        sObjectMgr->LoadMailLevelRewards();
        handler->SendGlobalGMSysMessage("DB table `mail_level_reward` reloaded.");
        return true;
    }

    static bool HandleReloadAuctionsCommand(ChatHandler* handler, const char* /*args*/)
    {
        ///- Reload dynamic data tables from the database
        sLog->outString("Re-Loading Auctions...");
        sAuctionMgr->LoadAuctionItems();
        sAuctionMgr->LoadAuctions();
        handler->SendGlobalGMSysMessage("Auctions reloaded.");
        return true;
    }

    static bool HandleReloadConditions(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Conditions...");
        sConditionMgr->LoadConditions(true);
        handler->SendGlobalGMSysMessage("Conditions reloaded.");
        return true;
    }

    static bool HandleReloadCreatureText(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Creature Texts...");
        sCreatureTextMgr->LoadCreatureTexts();
        handler->SendGlobalGMSysMessage("Creature Texts reloaded.");
        return true;
    }

    static bool HandleReloadSmartScripts(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Re-Loading Smart Scripts...");
        sSmartScriptMgr->LoadSmartAIFromDB();
        handler->SendGlobalGMSysMessage("Smart Scripts reloaded.");
        return true;
    }

    static bool HandleReloadVehicleAccessoryCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Reloading vehicle_accessory table...");
        sObjectMgr->LoadVehicleAccessories();
        handler->SendGlobalGMSysMessage("Vehicle accessories reloaded.");
        return true;
    }

    static bool HandleReloadVehicleTemplateAccessoryCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Reloading vehicle_template_accessory table...");
        sObjectMgr->LoadVehicleTemplateAccessories();
        handler->SendGlobalGMSysMessage("Vehicle template accessories reloaded.");
        return true;
    }

    static bool HandleReloadCreatureOnKillCurrencyCommand(ChatHandler* handler, const char* /*args*/)
    {
        sLog->outString("Reloading creature_onkill_currency table...");
        sObjectMgr->LoadCurrencyOnKill();
        handler->SendGlobalGMSysMessage("Creature onkill currency reloaded.");
        return true;
    }
};

void AddSC_reload_commandscript()
{
    new reload_commandscript();
}

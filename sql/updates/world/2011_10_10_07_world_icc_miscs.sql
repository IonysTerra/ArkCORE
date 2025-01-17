-- Eat Slime Puddle
REPLACE INTO `conditions` (`SourceTypeOrReferenceId`, `SourceEntry`, `ConditionTypeOrReference`, `ConditionValue1`, `ConditionValue2`, `Comment`) VALUES
('13', '70346', '18', '1', '37672', 'Mutated Abomination - Eat Slime Puddle'),
('13', '72456', '18', '1', '38285', 'Mutated Abomination - Eat Slime Puddle'),
('13', '72868', '18', '1', '37672', 'Mutated Abomination - Eat Slime Puddle'),
('13', '72869', '18', '1', '38285', 'Mutated Abomination - Eat Slime Puddle');

UPDATE `creature_template` SET `AIName` = 'PassiveAI', `flags_extra` = 2 WHERE `entry` IN (38309, 38308);
UPDATE `creature_template` SET `unit_flags` = 33554688 WHERE `entry` IN (36678, 38431, 38585, 38586);
DELETE FROM `creature_template_addon` WHERE `entry` IN (37127, 37134, 37132, 38125, 38126, 38131, 38132, 38133, 38130, 37122, 37123, 37124, 37125, 38298, 38299, 38303, 38304);

-- Stats for Broken Frostmourne
DELETE FROM `item_template` WHERE (`entry`=50840);
INSERT INTO `item_template` (`entry`, `class`, `subclass`, `unk0`, `name`, `displayid`, `Quality`, `Flags`, `FlagsExtra`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`, `requiredspell`, `requiredhonorrank`, `RequiredCityRank`, `RequiredReputationFaction`, `RequiredReputationRank`, `maxcount`, `stackable`, `ContainerSlots`, `StatsCount`, `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`, `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`, `stat_type6`, `stat_value6`, `stat_type7`, `stat_value7`, `stat_type8`, `stat_value8`, `stat_type9`, `stat_value9`, `stat_type10`, `stat_value10`, `ScalingStatDistribution`, `ScalingStatValue`, `dmg_min1`, `dmg_max1`, `dmg_type1`, `dmg_min2`, `dmg_max2`, `dmg_type2`, `armor`, `holy_res`, `fire_res`, `nature_res`, `frost_res`, `shadow_res`, `arcane_res`, `delay`, `ammo_type`, `RangedModRange`, `spellid_1`, `spelltrigger_1`, `spellcharges_1`, `spellppmRate_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`, `spellid_2`, `spelltrigger_2`, `spellcharges_2`, `spellppmRate_2`, `spellcooldown_2`, `spellcategory_2`, `spellcategorycooldown_2`, `spellid_3`, `spelltrigger_3`, `spellcharges_3`, `spellppmRate_3`, `spellcooldown_3`, `spellcategory_3`, `spellcategorycooldown_3`, `spellid_4`, `spelltrigger_4`, `spellcharges_4`, `spellppmRate_4`, `spellcooldown_4`, `spellcategory_4`, `spellcategorycooldown_4`, `spellid_5`, `spelltrigger_5`, `spellcharges_5`, `spellppmRate_5`, `spellcooldown_5`, `spellcategory_5`, `spellcategorycooldown_5`, `bonding`, `description`, `PageText`, `LanguageID`, `PageMaterial`, `startquest`, `lockid`, `Material`, `sheath`, `RandomProperty`, `RandomSuffix`, `block`, `itemset`, `MaxDurability`, `area`, `Map`, `BagFamily`, `TotemCategory`, `socketColor_1`, `socketContent_1`, `socketColor_2`, `socketContent_2`, `socketColor_3`, `socketContent_3`, `socketBonus`, `GemProperties`, `RequiredDisenchantSkill`, `ArmorDamageModifier`, `Duration`, `ItemLimitCategory`, `HolidayId`, `ScriptName`, `DisenchantID`, `FoodType`, `minMoneyLoot`, `maxMoneyLoot`, `WDBVerified`) VALUES 
(50840, 2, 7, -1, 'Monster - Frostmourne, Broken (Arthas Only Special)', 64086, 4, 0, 0, 0, 0, 0, 21, -1, -1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 4, 4, 14, 3, 20, 7, 10, 14, 20, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 20, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2900, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, '', 0, 0, 0, 0, 0, 1, 3, 0, 0, 0, 0, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 'internalitemhanler', 0, 0, 0, 0, 1);
UPDATE `item_template` SET `InventoryType`='17', `sheath`='0', `BuyCount`='1' WHERE `entry`='50840';

-- FoF SpellScript
DELETE FROM `spell_script_names` WHERE (`spell_id` = 72350);
INSERT INTO `spell_script_names` VALUES
(72350, 'spell_lich_king_fury_of_frostmourne');

-- Lich King: Soul Barrage should make him fly for outro
DELETE FROM `spell_linked_spell` WHERE (`spell_trigger` = 72305);
INSERT INTO `spell_linked_spell` VALUES
(72305, 34873, 2, 'Soul Barrage linked to Fly');

-- Fix Quake Visual (DO NOT EXECUTE MORE THAN ONCE!!!)
-- UPDATE creature SET position_z = position_z + 1 WHERE id = 22515 AND map = 631;
INSERT IGNORE INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ErrorTextId`, `ScriptName`, `Comment`) values(13, 0, 72262, 0, 18, 1, 22515, 0, 0,'','Quake visual');

-- Cleaning console error in Valithria Dreamwalker's Rage fixes.
DELETE FROM `conditions` WHERE `SourceEntry` = 71189;
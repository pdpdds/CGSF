--
-- Modify the table `mana_characters` to add the slot field.
--

ALTER TABLE `mana_characters` ADD COLUMN `slot` smallint(5) unsigned NOT NULL DEFAULT '0';

-- Update database version.
UPDATE mana_world_states
SET value = '14',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version';

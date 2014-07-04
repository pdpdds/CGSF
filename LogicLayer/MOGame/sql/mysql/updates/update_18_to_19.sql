ALTER TABLE mana_item_instances CHANGE amount amount int(10) unsigned NOT NULL;

-- Update database version.
UPDATE mana_world_states
SET value = '19',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version';

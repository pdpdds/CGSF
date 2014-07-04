ALTER TABLE mana_characters CHANGE map_id map_id smallint(5);

-- Update database version.
UPDATE mana_world_states
SET value = '16',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version'; 

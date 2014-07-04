START TRANSACTION;

-- Set existing world variables to the world map (0)
UPDATE mana_world_states SET map_id = 0 WHERE map_id IS NULL;

-- Set known system variables to the system map (-1)
UPDATE mana_world_states SET map_id = -1 WHERE
    state_name = 'database_version' OR
    state_name = 'accountserver_version' OR
    state_name = 'accountserver_startup';

-- Fix up the world states table to disallow map_id to be NULL, and to have
-- the correct primary key.
ALTER TABLE mana_world_states MODIFY map_id INTEGER NOT NULL;
ALTER TABLE mana_world_states MODIFY value TEXT NOT NULL;
ALTER TABLE mana_world_states DROP PRIMARY KEY;
ALTER TABLE mana_world_states ADD PRIMARY KEY (state_name, map_id);

-- Update database version.
UPDATE mana_world_states
    SET value = '20',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;

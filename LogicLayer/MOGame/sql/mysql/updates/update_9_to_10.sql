-- This was only a fix for the SQLite database

UPDATE mana_world_states SET value = '10' WHERE state_name = 'database_version';


-- Add the slot column to table mana_characters
ALTER TABLE mana_characters ADD slot INTEGER NOT NULL DEFAULT (0);

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '14',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';


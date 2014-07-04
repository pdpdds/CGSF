BEGIN;

ALTER TABLE mana_char_specials ADD COLUMN special_current_mana INTEGER DEFAULT 0 NOT NULL;

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '21',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;

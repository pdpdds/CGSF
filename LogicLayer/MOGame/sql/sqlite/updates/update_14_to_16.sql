-- No changes required. It was only an issue with mysql.

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '16',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

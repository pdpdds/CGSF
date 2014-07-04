-- No change required. It was an mysql issue only


-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '19',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


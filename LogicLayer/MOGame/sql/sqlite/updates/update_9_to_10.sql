DROP INDEX IF EXISTS mana_char_kill_stats_chars;
DROP INDEX IF EXISTS mana_char_kill_stats_char;
CREATE INDEX mana_char_kill_stats_char on mana_char_kill_stats ( char_id );

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '10',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


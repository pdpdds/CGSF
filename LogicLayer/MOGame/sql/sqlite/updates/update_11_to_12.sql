--
-- SQLite - update 11 to 12 doesn't affect Sqlite,
-- so we only change the database version number.
--

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '12',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


-- add two columns to table tmw_accounts
ALTER TABLE tmw_accounts ADD authorization TEXT NULL;
ALTER TABLE tmw_accounts ADD expiration    INTEGER NULL;

-- update the database version, and set date of update
UPDATE tmw_world_states
   SET value      = '4',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


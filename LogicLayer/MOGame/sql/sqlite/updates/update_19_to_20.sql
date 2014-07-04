--
-- SQLite does not support altering of columns, so we'll need to recreate the
-- table and copy the data over.
--

BEGIN;

CREATE TEMPORARY TABLE mana_world_states_backup
(
   state_name   TEXT        PRIMARY KEY,
   map_id       INTEGER     NULL,
   value        TEXT        NULL,
   moddate      INTEGER     NOT NULL
);

INSERT INTO mana_world_states_backup SELECT
    state_name, map_id, value, moddate FROM mana_world_states;

DROP TABLE mana_world_states;

-- Create the new world states table with the corrected primary key, and move
-- the existing data over
CREATE TABLE mana_world_states
(
   state_name   TEXT        NOT NULL,
   map_id       INTEGER     NOT NULL,
   value        TEXT        NOT NULL,
   moddate      INTEGER     NOT NULL,
   --
   PRIMARY KEY (state_name, map_id)
);

-- Copy over all map states
INSERT INTO mana_world_states (state_name, map_id, value, moddate)
    SELECT state_name, map_id, value, moddate
      FROM mana_world_states_backup
      WHERE map_id > 0;

-- Copy over all world states
INSERT INTO mana_world_states (state_name, map_id, value, moddate)
    SELECT state_name, 0, value, moddate
      FROM mana_world_states_backup
      WHERE map_id ISNULL;

-- Move some known system variables into the system scope
UPDATE mana_world_states SET map_id = -1 WHERE
    state_name = 'database_version' OR
    state_name = 'accountserver_version' OR
    state_name = 'accountserver_startup';

DROP TABLE mana_world_states_backup;

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '20',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;

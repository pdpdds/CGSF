
-- create table mana_char_kill_stats

CREATE TABLE mana_char_kill_stats
(
    char_id     INTEGER     NOT NULL,
    monster_id  INTEGER     NOT NULL,
    kills       INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_kill_stats_char on mana_char_kill_stats ( char_id );

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '8',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


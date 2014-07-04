
-- create table tmw_char_status_effects

CREATE TABLE tmw_char_status_effects
(
    char_id     INTEGER     NOT NULL,
    status_id   INTEGER     NOT NULL,
    status_time INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES tmw_characters(id)
);

CREATE INDEX tmw_char_status_char on tmw_char_status_effects ( char_id );

-- update the database version, and set date of update
UPDATE tmw_world_states
   SET value      = '6',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


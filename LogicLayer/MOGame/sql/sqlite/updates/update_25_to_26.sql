BEGIN;

CREATE TABLE mana_questlog
(
    char_id           INTEGER     NOT NULL,
    quest_id          INTEGER     NOT NULL,
    quest_state       INTEGER     NOT NULL,
    quest_title       TEXT        NOT NULL,
    quest_description TEXT        NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);
CREATE INDEX mana_questlog_char_id ON mana_questlog ( char_id );
CREATE INDEX mana_questlog_quest_id ON mana_questlog ( quest_id );

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '26',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;

BEGIN;

CREATE TABLE mana_char_abilities
(
    char_id                 INTEGER     NOT NULL,
    ability_id              INTEGER     NOT NULL,
    ability_current_points  INTEGER     NOT NULL,
    PRIMARY KEY (char_id, ability_id),
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_abilities_char on mana_char_abilities ( char_id );

INSERT INTO mana_char_abilities (char_id, ability_id, ability_current_points)
    SELECT char_id, special_id, special_current_mana FROM mana_char_specials;

DROP TABLE mana_char_specials;


-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '22',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;

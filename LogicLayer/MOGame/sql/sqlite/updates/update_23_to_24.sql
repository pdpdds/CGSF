BEGIN;

CREATE TABLE mana_char_abilities_backup
(
    char_id                 INTEGER     NOT NULL,
    ability_id              INTEGER     NOT NULL,
    PRIMARY KEY (char_id, ability_id),
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

INSERT INTO mana_char_abilities_backup SELECT char_id, ability_id FROM mana_char_abilities;

DROP TABLE mana_char_abilities;

CREATE TABLE mana_char_abilities
(
    char_id                 INTEGER     NOT NULL,
    ability_id              INTEGER     NOT NULL,
    PRIMARY KEY (char_id, ability_id),
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

INSERT INTO mana_char_abilities SELECT char_id, ability_id FROM mana_char_abilities_backup;

DROP TABLE mana_char_abilities_backup;

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '24',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;


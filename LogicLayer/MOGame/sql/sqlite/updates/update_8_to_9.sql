CREATE TABLE mana_char_specials
(
    char_id     INTEGER     NOT NULL,
    special_id  INTEGER     NOT NULL,
	PRIMARY KEY (char_id, special_id),
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_specials_char on mana_char_specials ( char_id );

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '9',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


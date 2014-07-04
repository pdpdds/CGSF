BEGIN;

-- There is no way to convert all your skills to attributes. You will have to
-- do this manually.

CREATE TABLE mana_characters_backup
(
   id           INTEGER     PRIMARY KEY,
   user_id      INTEGER     NOT NULL,
   name         TEXT        NOT NULL UNIQUE,
   gender       INTEGER     NOT NULL,
   hair_style   INTEGER     NOT NULL,
   hair_color   INTEGER     NOT NULL,
   char_pts     INTEGER     NOT NULL,
   correct_pts  INTEGER     NOT NULL,
   x            INTEGER     NOT NULL,
   y            INTEGER     NOT NULL,
   map_id       INTEGER     NOT NULL,
   slot         INTEGER     NOT NULL,
   --  
   FOREIGN KEY (user_id) REFERENCES mana_accounts(id)
);

INSERT INTO mana_characters_backup SELECT
    id, user_id, name, gender, hair_style, hair_color, char_pts, correct_pts,
    x, y, map_id, slot FROM mana_characters;

DROP TABLE mana_characters;

CREATE TABLE mana_characters
(
   id           INTEGER     PRIMARY KEY,
   user_id      INTEGER     NOT NULL,
   name         TEXT        NOT NULL UNIQUE,
   gender       INTEGER     NOT NULL,
   hair_style   INTEGER     NOT NULL,
   hair_color   INTEGER     NOT NULL,
   char_pts     INTEGER     NOT NULL,
   correct_pts  INTEGER     NOT NULL,
   x            INTEGER     NOT NULL,
   y            INTEGER     NOT NULL,
   map_id       INTEGER     NOT NULL,
   slot         INTEGER     NOT NULL,
   --  
   FOREIGN KEY (user_id) REFERENCES mana_accounts(id)
);

INSERT INTO mana_characters SELECT * FROM mana_characters_backup;
DROP TABLE mana_characters_backup;

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '23',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;


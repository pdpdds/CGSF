--
-- SQLite does not support removing of columns, so we'll need to recreate the
-- table and copy the data over.
--

BEGIN TRANSACTION;

CREATE TEMPORARY TABLE mana_characters_backup
(
   id           INTEGER     PRIMARY KEY,
   user_id      INTEGER     NOT NULL,
   name         TEXT        NOT NULL UNIQUE,
   gender       INTEGER     NOT NULL,
   hair_style   INTEGER     NOT NULL,
   hair_color   INTEGER     NOT NULL,
   level        INTEGER     NOT NULL,
   char_pts     INTEGER     NOT NULL,
   correct_pts  INTEGER     NOT NULL,
   x            INTEGER     NOT NULL,
   y            INTEGER     NOT NULL,
   map_id       INTEGER     NOT NULL,
   --
   FOREIGN KEY (user_id) REFERENCES mana_accounts(id)
);

INSERT INTO mana_characters_backup SELECT
    id,
    user_id,
    name,
    gender,
    hair_style,
    hair_color,
    level,
    char_pts,
    correct_pts,
    x,
    y,
    map_id FROM mana_characters;

DROP TABLE mana_characters;

CREATE TABLE mana_characters
(
   id           INTEGER     PRIMARY KEY,
   user_id      INTEGER     NOT NULL,
   name         TEXT        NOT NULL UNIQUE,
   gender       INTEGER     NOT NULL,
   hair_style   INTEGER     NOT NULL,
   hair_color   INTEGER     NOT NULL,
   level        INTEGER     NOT NULL,
   char_pts     INTEGER     NOT NULL,
   correct_pts  INTEGER     NOT NULL,
   x            INTEGER     NOT NULL,
   y            INTEGER     NOT NULL,
   map_id       INTEGER     NOT NULL,
   --
   FOREIGN KEY (user_id) REFERENCES mana_accounts(id)
);

CREATE INDEX mana_characters_user ON mana_characters ( user_id );
CREATE UNIQUE INDEX mana_characters_name ON mana_characters ( name );

INSERT INTO mana_characters SELECT
    id,
    user_id,
    name,
    gender,
    hair_style,
    hair_color,
    level,
    char_pts,
    correct_pts,
    x,
    y,
    map_id FROM mana_characters_backup;

DROP TABLE mana_characters_backup;


CREATE TABLE IF NOT EXISTS mana_char_attr
(
   char_id      INTEGER     NOT NULL,
   attr_id      INTEGER     NOT NULL,
   attr_base    FLOAT       NOT NULL,
   attr_mod     FLOAT       NOT NULL,
   --
   FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX IF NOT EXISTS mana_char_attr_char ON mana_char_attr ( char_id );

CREATE TABLE IF NOT EXISTS mana_char_equips
(
    id               INTEGER    PRIMARY KEY,
    owner_id         INTEGER    NOT NULL,
    slot_type        INTEGER    NOT NULL,
    inventory_slot   INTEGER    NOT NULL,
    --
    FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '11',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';

COMMIT;

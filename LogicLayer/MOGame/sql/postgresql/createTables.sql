--
-- The Mana Server
-- Copyright (C) 2009  The Mana World Development Team
--
-- This file is part of The Mana Server.
--
-- The Mana Server is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- any later version.
--
-- The Mana Server is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
--

CREATE TABLE mana_accounts
(
   id           SERIAL      PRIMARY KEY,
   username     TEXT        NOT NULL UNIQUE,
   password     TEXT        NOT NULL,
   email        TEXT        NOT NULL,
   level        SMALLINT    NOT NULL,
   banned       SMALLINT    NOT NULL,
   registration INTEGER     NOT NULL,
   lastlogin    INTEGER     NOT NULL
);

CREATE INDEX mana_accounts_username ON mana_accounts ( username );


CREATE TABLE mana_characters
(
   id           SERIAL      PRIMARY KEY,
   user_id      INTEGER     NOT NULL,
   name         TEXT        NOT NULL UNIQUE,
   gender       SMALLINT    NOT NULL,
   hair_style   SMALLINT    NOT NULL,
   hair_color   INTEGER     NOT NULL,
   level        INTEGER     NOT NULL,
   char_pts     INTEGER     NOT NULL,
   correct_pts  INTEGER     NOT NULL,
   money        INTEGER     NOT NULL,
   x            SMALLINT    NOT NULL,
   y            SMALLINT    NOT NULL,
   map_id       SMALLINT    NOT NULL,
   str          SMALLINT    NOT NULL,
   agi          SMALLINT    NOT NULL,
   dex          SMALLINT    NOT NULL,
   vit          SMALLINT    NOT NULL,
   int          SMALLINT    NOT NULL,
   will         SMALLINT    NOT NULL,
   unarmed_exp  INTEGER     NOT NULL,
   knife_exp    INTEGER     NOT NULL,
   sword_exp    INTEGER     NOT NULL,
   polearm_exp  INTEGER     NOT NULL,
   staff_exp    INTEGER     NOT NULL,
   whip_exp     INTEGER     NOT NULL,
   bow_exp      INTEGER     NOT NULL,
   shoot_exp    INTEGER     NOT NULL,
   mace_exp     INTEGER     NOT NULL,
   axe_exp      INTEGER     NOT NULL,
   thrown_exp   INTEGER     NOT NULL,
   --
   FOREIGN KEY (user_id) REFERENCES mana_accounts(id)
);

CREATE TABLE mana_inventories
(
   id           SERIAL      PRIMARY KEY,
   owner_id     INTEGER     NOT NULL,
   slot         SMALLINT    NOT NULL,
   class_id     INTEGER     NOT NULL,
   amount       SMALLINT    NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

CREATE TABLE mana_guilds
(
   id           SERIAL      PRIMARY KEY,
   name         TEXT        NOT NULL UNIQUE
);

CREATE TABLE mana_guild_members
(
   guild_id     INTEGER     NOT NULL,
   member_id    INTEGER     NOT NULL,
   rights       INTEGER     NOT NULL,
   --
   FOREIGN KEY (guild_id)  REFERENCES mana_guilds(id),
   FOREIGN KEY (member_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_guild_members_g ON mana_guild_members ( guild_id );
CREATE INDEX mana_guild_members_m ON mana_guild_members ( member_id );

CREATE TABLE mana_quests
(
   owner_id     INTEGER     NOT NULL,
   name         TEXT        NOT NULL,
   value        TEXT        NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

CREATE TABLE mana_world_states
(
   state_name   TEXT        NOT NULL,
   map_id       INTEGER     NOT NULL,
   value        TEXT        NOT NULL,
   moddate      INTEGER     NOT NULL,
   PRIMARY KEY (`state_name`, `map_id`)
);

INSERT INTO "mana_world_states" VALUES('accountserver_startup',-1,'0',1221633910);
INSERT INTO "mana_world_states" VALUES('accountserver_version',-1,'0',1221633910);
INSERT INTO "mana_world_states" VALUES('database_version',-1,'20',1221633910);

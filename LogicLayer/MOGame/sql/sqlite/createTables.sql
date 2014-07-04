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

BEGIN;

-----------------------------------------------------------------------------
-- Tables
-----------------------------------------------------------------------------

CREATE TABLE mana_accounts
(
   id            INTEGER     PRIMARY KEY,
   username      TEXT        NOT NULL UNIQUE,
   password      TEXT        NOT NULL,
   email         TEXT        NOT NULL,
   level         INTEGER     NOT NULL,
   banned        INTEGER     NOT NULL,
   registration  INTEGER     NOT NULL,
   lastlogin     INTEGER     NOT NULL,
   authorization TEXT            NULL,
   expiration    INTEGER         NULL
);

CREATE UNIQUE INDEX mana_accounts_username ON mana_accounts ( username );
CREATE UNIQUE INDEX mana_accounts_email    ON mana_accounts ( email );

-----------------------------------------------------------------------------

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

CREATE INDEX mana_characters_user ON mana_characters ( user_id );
CREATE UNIQUE INDEX mana_characters_name ON mana_characters ( name );

-----------------------------------------------------------------------------

CREATE TABLE mana_char_attr
(
   char_id      INTEGER     NOT NULL,
   attr_id      INTEGER     NOT NULL,
   attr_base    FLOAT       NOT NULL,
   attr_mod     FLOAT       NOT NULL,
   --
   FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_attr_char ON mana_char_attr ( char_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_char_status_effects
(
    char_id     INTEGER     NOT NULL,
    status_id   INTEGER     NOT NULL,
    status_time INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_status_char on mana_char_status_effects ( char_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_char_kill_stats
(
    char_id     INTEGER     NOT NULL,
    monster_id  INTEGER     NOT NULL,
    kills       INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_kill_stats_char on mana_char_kill_stats ( char_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_char_abilities
(
    char_id                 INTEGER     NOT NULL,
    ability_id              INTEGER     NOT NULL,
    PRIMARY KEY (char_id, ability_id),
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_char_abilities_char on mana_char_abilities ( char_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_items
(
    id           INTEGER    PRIMARY KEY,
    name         TEXT       NOT NULL,
    description  TEXT       NOT NULL,
    image        TEXT       NOT NULL,
    weight       INTEGER    NOT NULL,
    itemtype     TEXT       NOT NULL,
    effect       TEXT,
    dyestring    TEXT
);

CREATE INDEX mana_items_type ON mana_items (itemtype);

-----------------------------------------------------------------------------

CREATE TABLE mana_item_instances
(
    item_id       INTEGER    PRIMARY KEY,
    itemclass_id  INTEGER    NOT NULL,
    amount        INTEGER    NOT NULL,
    --
    FOREIGN KEY (itemclass_id) REFERENCES mana_items(id)
);

CREATE INDEX mana_item_instances_typ ON mana_item_instances ( itemclass_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_item_attributes
(
    attribute_id    INTEGER    PRIMARY KEY,
    item_id         INTEGER    NOT NULL,
    attribute_class INTEGER    NOT NULL,
    attribute_value TEXT,
    --
    FOREIGN KEY (item_id) REFERENCES mana_item_instances(item_id)
);

CREATE INDEX mana_item_attributes_item ON mana_item_attributes ( item_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_floor_items
(
    id            INTEGER     PRIMARY KEY,
    map_id        INTEGER     NOT NULL,
    item_id       INTEGER     NOT NULL,
    amount        INTEGER     NOT NULL,
    pos_x         INTEGER     NOT NULL,
    pos_y         INTEGER     NOT NULL
);

-----------------------------------------------------------------------------

-- todo: remove class_id and amount and reference on mana_item_instances
CREATE TABLE mana_inventories
(
   id           INTEGER     PRIMARY KEY,
   owner_id     INTEGER     NOT NULL,
   slot         INTEGER     NOT NULL,
   class_id     INTEGER     NOT NULL,
   amount       INTEGER     NOT NULL,
   equipped     INTEGER     NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_inventories_owner ON mana_inventories ( owner_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_guilds
(
   id           INTEGER     PRIMARY KEY,
   name         TEXT        NOT NULL UNIQUE
);

-----------------------------------------------------------------------------

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

-----------------------------------------------------------------------------

CREATE TABLE mana_quests
(
   owner_id     INTEGER     NOT NULL,
   name         TEXT        NOT NULL,
   value        TEXT        NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

-----------------------------------------------------------------------------

CREATE TABLE mana_world_states
(
   state_name   TEXT        NOT NULL,
   map_id       INTEGER     NOT NULL,
   value        TEXT        NOT NULL,
   moddate      INTEGER     NOT NULL,
   --
   PRIMARY KEY (state_name, map_id)
);

-----------------------------------------------------------------------------

CREATE TABLE mana_auctions
(
   auction_id    INTEGER     PRIMARY KEY,
   auction_state INTEGER     NOT NULL,
   char_id       INTEGER     NOT NULL,
   itemclass_id  INTEGER     NOT NULL,
   amount        INTEGER     NOT NULL,
   start_time    INTEGER     NOT NULL,
   end_time      INTEGER     NOT NULL,
   start_price   INTEGER     NOT NULL,
   min_price     INTEGER,
   buyout_price  INTEGER,
   description   TEXT,
   --
   FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_auctions_owner ON mana_auctions ( char_id );
CREATE INDEX mana_auctions_state ON mana_auctions ( auction_state );
CREATE INDEX mana_auctions_item  ON mana_auctions ( itemclass_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_auction_bids
(
   bid_id        INTEGER     PRIMARY KEY,
   auction_id    INTEGER     NOT NULL,
   char_id       INTEGER     NOT NULL,
   bid_time      INTEGER     NOT NULL,
   bid_price     INTEGER     NOT NULL,
   --
   FOREIGN KEY (auction_id) REFERENCES mana_auctions(auction_id),
   FOREIGN KEY (char_id)    REFERENCES mana_characters(id)
);

CREATE INDEX mana_auction_bids_auction ON mana_auction_bids ( auction_id );
CREATE INDEX mana_auction_bids_owner   ON mana_auction_bids ( char_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_post
(
   letter_id        INTEGER   PRIMARY KEY,
   sender_id        INTEGER   NOT NULL,
   receiver_id      INTEGER   NOT NULL,
   letter_type      INTEGER   NOT NULL,
   expiration_date  INTEGER   NOT NULL,
   sending_date     INTEGER   NOT NULL,
   letter_text      TEXT          NULL,
   --
   FOREIGN KEY (sender_id)   REFERENCES mana_characters(id),
   FOREIGN KEY (receiver_id) REFERENCES mana_characters(id)
);

CREATE INDEX mana_post_sender   ON mana_post ( sender_id );
CREATE INDEX mana_post_receiver ON mana_post ( receiver_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_post_attachments
(
   attachment_id    INTEGER   PRIMARY KEY,
   letter_id        INTEGER   NOT NULL,
   item_id          INTEGER   NOT NULL,
   --
   FOREIGN KEY (letter_id) REFERENCES mana_post(letter_id),
   FOREIGN KEY (item_id)   REFERENCES mana_item_instances(item_id)
);

CREATE INDEX mana_post_attachments_ltr ON mana_post_attachments ( letter_id );
CREATE INDEX mana_post_attachments_itm ON mana_post_attachments ( item_id );

-----------------------------------------------------------------------------

CREATE TABLE mana_transaction_codes
(
   id            INTEGER     PRIMARY KEY,
   description   TEXT        NOT NULL,
   category      TEXT        NOT NULL
);

CREATE INDEX mana_transaction_codes_cat    ON mana_transaction_codes ( category );

-----------------------------------------------------------------------------

CREATE TABLE mana_transactions
(
    id          INTEGER     PRIMARY KEY,
    char_id     INTEGER     NOT NULL,
    action      INTEGER     NOT NULL,
    message     TEXT,
    time        INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id),
    FOREIGN KEY (action)  REFERENCES mana_transaction_codes(id)
);

CREATE INDEX mana_transactions_char    ON mana_transactions ( char_id );
CREATE INDEX mana_transactions_action  ON mana_transactions ( action );
CREATE INDEX mana_transactions_time    ON mana_transactions ( time );

-----------------------------------------------------------------------------

CREATE TABLE mana_online_list
(
    char_id     INTEGER     PRIMARY KEY,
    login_date  INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES mana_characters(id)
);

-----------------------------------------------------------------------------

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

-----------------------------------------------------------------------------
-- Views
-----------------------------------------------------------------------------

-- List all online users

CREATE VIEW mana_v_online_chars
AS
   SELECT l.char_id    as char_id,
          l.login_date as login_date,
          c.user_id    as user_id,
          c.name       as name,
          c.gender     as gender,
          c.map_id     as map_id
     FROM mana_online_list l
     JOIN mana_characters c
       ON l.char_id = c.id;


-- Show all stored transactions

CREATE VIEW mana_v_transactions
AS
   SELECT t.id           as transaction_id,
          t.time         as transacition_time,
          a.id           as user_id,
          a.username     as username,
          c.id           as char_id,
          c.name         as charname,
          tc.id          as action_id,
          tc.description as action,
          tc.category    as category,
          t.message      as message
     FROM mana_transactions t
     JOIN mana_characters c
       ON t.char_id = c.id
     JOIN mana_accounts a
       ON c.user_id = a.id
     JOIN mana_transaction_codes tc
       ON t.action = tc.id;

-----------------------------------------------------------------------------
-- Initial data
-----------------------------------------------------------------------------

-- initial world states and database version

INSERT INTO mana_world_states VALUES('accountserver_startup',-1,'0', strftime('%s','now'));
INSERT INTO mana_world_states VALUES('accountserver_version',-1,'0', strftime('%s','now'));
INSERT INTO mana_world_states VALUES('database_version',     -1,'26', strftime('%s','now'));

-- all known transaction codes

INSERT INTO mana_transaction_codes VALUES (  1, 'Character created',        'Character' );
INSERT INTO mana_transaction_codes VALUES (  2, 'Character selected',       'Character' );
INSERT INTO mana_transaction_codes VALUES (  3, 'Character deleted',        'Character' );
INSERT INTO mana_transaction_codes VALUES (  4, 'Public message sent',      'Chat' );
INSERT INTO mana_transaction_codes VALUES (  5, 'Public message annouced',  'Chat' );
INSERT INTO mana_transaction_codes VALUES (  6, 'Private message sent',     'Chat' );
INSERT INTO mana_transaction_codes VALUES (  7, 'Channel joined',           'Chat' );
INSERT INTO mana_transaction_codes VALUES (  8, 'Channel kicked',           'Chat' );
INSERT INTO mana_transaction_codes VALUES (  9, 'Channel MODE',             'Chat' );
INSERT INTO mana_transaction_codes VALUES ( 10, 'Channel QUIT',             'Chat' );
INSERT INTO mana_transaction_codes VALUES ( 11, 'Channel LIST',             'Chat' );
INSERT INTO mana_transaction_codes VALUES ( 12, 'Channel USERLIST',         'Chat' );
INSERT INTO mana_transaction_codes VALUES ( 13, 'Channel TOPIC',            'Chat' );
INSERT INTO mana_transaction_codes VALUES ( 14, 'Command BAN',              'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 15, 'Command DROP',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 16, 'Command ITEM',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 17, 'Command MONEY',            'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 18, 'Command SETGROUP',         'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 19, 'Command SPAWN',            'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 20, 'Command WARP',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 21, 'Item picked up',           'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 22, 'Item used',                'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 23, 'Item dropped',             'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 24, 'Item moved',               'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 25, 'Target attacked',          'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 26, 'ACTION Changed',           'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 27, 'Trade requested',          'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 28, 'Trade ended',              'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 29, 'Trade money',              'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 30, 'Trade items',              'Actions' );
INSERT INTO mana_transaction_codes VALUES ( 31, 'Attribute increased',      'Character' );
INSERT INTO mana_transaction_codes VALUES ( 32, 'Attribute decreased',      'Character' );
INSERT INTO mana_transaction_codes VALUES ( 33, 'Command MUTE',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 35, 'Command INVISIBLE',        'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 36, 'Command COMBAT',           'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 37, 'Command ANNOUNCE',         'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 38, 'Command ANNOUNCE_LOCAL',   'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 39, 'Command KILL',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 40, 'Command FX',               'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 41, 'Command LOG',              'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 42, 'Command KILLMONSTER',      'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 43, 'Command GOTO',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 44, 'Command GONEXT',           'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 45, 'Command GOPREV',           'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 46, 'Command IPBAN',            'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 47, 'Command WIPE_ITEMS',       'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 48, 'Command WIPE_LEVEL',       'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 49, 'Command SHUTDOWN_THIS',    'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 50, 'Command SHUTDOWN_ALL',     'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 51, 'Command RESTART_THIS',     'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 52, 'Command RESTART_ALL',      'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 53, 'Command ATTRIBUTE',        'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 54, 'Command KICK',             'Commands' );

END;

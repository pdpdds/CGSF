
-- rename tables to new prefix

ALTER TABLE tmw_accounts RENAME TO mana_accounts;
ALTER TABLE tmw_characters RENAME TO mana_characters;
ALTER TABLE tmw_char_skills RENAME TO mana_char_skills;
ALTER TABLE tmw_char_status_effects RENAME TO mana_char_status_effects;
ALTER TABLE tmw_items RENAME TO mana_items;
ALTER TABLE tmw_item_instances RENAME TO mana_item_instances;
ALTER TABLE tmw_item_attributes RENAME TO mana_item_attributes;
ALTER TABLE tmw_inventories RENAME TO mana_inventories;
ALTER TABLE tmw_guilds RENAME TO mana_guilds;
ALTER TABLE tmw_guild_members RENAME TO mana_guild_members;
ALTER TABLE tmw_quests RENAME TO mana_quests;
ALTER TABLE tmw_world_states RENAME TO mana_world_states;
ALTER TABLE tmw_auctions RENAME TO mana_auctions;
ALTER TABLE tmw_auction_bids RENAME TO mana_auction_bids;
ALTER TABLE tmw_post RENAME TO mana_post;
ALTER TABLE tmw_post_attachments RENAME TO mana_post_attachments;
ALTER TABLE tmw_transaction_codes RENAME TO mana_transaction_codes;
ALTER TABLE tmw_transactions RENAME TO mana_transactions;
ALTER TABLE tmw_online_list RENAME TO mana_online_list;

-- rename indexes (apparently have to drop and recreate)

DROP INDEX tmw_accounts_username;
DROP INDEX tmw_accounts_email;
DROP INDEX tmw_characters_user;
DROP INDEX tmw_characters_name;
DROP INDEX tmw_char_skills_char;
DROP INDEX tmw_char_status_char;
DROP INDEX tmw_items_type;
DROP INDEX tmw_item_instances_typ;
DROP INDEX tmw_item_attributes_item;
DROP INDEX tmw_inventories_owner;
DROP INDEX tmw_guild_members_g;
DROP INDEX tmw_guild_members_m;
DROP INDEX tmw_auctions_owner;
DROP INDEX tmw_auctions_state;
DROP INDEX tmw_auctions_item;
DROP INDEX tmw_auction_bids_auction;
DROP INDEX tmw_auction_bids_owner;
DROP INDEX tmw_post_sender;
DROP INDEX tmw_post_receiver;
DROP INDEX tmw_post_attachments_ltr;
DROP INDEX tmw_post_attachments_itm;
DROP INDEX tmw_transaction_codes_cat;
DROP INDEX tmw_transactions_char;
DROP INDEX tmw_transactions_action;
DROP INDEX tmw_transactions_time;

CREATE UNIQUE INDEX mana_accounts_username ON mana_accounts ( username );
CREATE UNIQUE INDEX mana_accounts_email    ON mana_accounts ( email );
CREATE INDEX mana_characters_user ON mana_characters ( user_id );
CREATE UNIQUE INDEX mana_characters_name ON mana_characters ( name );
CREATE INDEX mana_char_skills_char ON mana_char_skills ( char_id );
CREATE INDEX mana_char_status_char on mana_char_status_effects ( char_id );
CREATE INDEX mana_items_type ON mana_items (itemtype);
CREATE INDEX mana_item_instances_typ ON mana_item_instances ( itemclass_id );
CREATE INDEX mana_item_attributes_item ON mana_item_attributes ( item_id );
CREATE INDEX mana_inventories_owner ON mana_inventories ( owner_id );
CREATE INDEX mana_guild_members_g ON mana_guild_members ( guild_id );
CREATE INDEX mana_guild_members_m ON mana_guild_members ( member_id );
CREATE INDEX mana_auctions_owner ON mana_auctions ( char_id );
CREATE INDEX mana_auctions_state ON mana_auctions ( auction_state );
CREATE INDEX mana_auctions_item  ON mana_auctions ( itemclass_id );
CREATE INDEX mana_auction_bids_auction ON mana_auction_bids ( auction_id );
CREATE INDEX mana_auction_bids_owner   ON mana_auction_bids ( char_id );
CREATE INDEX mana_post_sender   ON mana_post ( sender_id );
CREATE INDEX mana_post_receiver ON mana_post ( receiver_id );
CREATE INDEX mana_post_attachments_ltr ON mana_post_attachments ( letter_id );
CREATE INDEX mana_post_attachments_itm ON mana_post_attachments ( item_id );
CREATE INDEX mana_transaction_codes_cat    ON mana_transaction_codes ( category );
CREATE INDEX mana_transactions_char    ON mana_transactions ( char_id );
CREATE INDEX mana_transactions_action  ON mana_transactions ( action );
CREATE INDEX mana_transactions_time    ON mana_transactions ( time );

-- rename views (have to drop and recreate as well)

DROP VIEW tmw_v_online_chars;
DROP VIEW tmw_v_transactions;

CREATE VIEW mana_v_online_chars
AS
   SELECT l.char_id    as char_id,
          l.login_date as login_date,
          c.user_id    as user_id,
          c.name       as name,
          c.gender     as gender,
          c.level      as level,
          c.map_id     as map_id
     FROM mana_online_list l
     JOIN mana_characters c
       ON l.char_id = c.id;

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


-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '7',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


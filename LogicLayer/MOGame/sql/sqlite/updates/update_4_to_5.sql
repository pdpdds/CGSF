
-- create table tmw_transaction_codes
CREATE TABLE tmw_transaction_codes
(
   id            INTEGER     PRIMARY KEY,
   description   TEXT        NOT NULL,
   category      TEXT        NOT NULL
);

CREATE INDEX tmw_transaction_codes_cat    ON tmw_transaction_codes ( category );

INSERT INTO tmw_transaction_codes VALUES (  1, 'Character created',     'Character' );
INSERT INTO tmw_transaction_codes VALUES (  2, 'Character selected',    'Character' );
INSERT INTO tmw_transaction_codes VALUES (  3, 'Character deleted',     'Character' );

INSERT INTO tmw_transaction_codes VALUES (  4, 'Public message sent',      'Chat' );
INSERT INTO tmw_transaction_codes VALUES (  5, 'Public message annouced',  'Chat' );
INSERT INTO tmw_transaction_codes VALUES (  6, 'Private message sent',     'Chat' );
INSERT INTO tmw_transaction_codes VALUES (  7, 'Channel joined',           'Chat' );
INSERT INTO tmw_transaction_codes VALUES (  8, 'Channel kicked',           'Chat' );
INSERT INTO tmw_transaction_codes VALUES (  9, 'Channel MODE',             'Chat' );
INSERT INTO tmw_transaction_codes VALUES ( 10, 'Channel QUIT',             'Chat' );
INSERT INTO tmw_transaction_codes VALUES ( 11, 'Channel LIST',             'Chat' );
INSERT INTO tmw_transaction_codes VALUES ( 12, 'Channel USERLIST',         'Chat' );
INSERT INTO tmw_transaction_codes VALUES ( 13, 'Channel TOPIC',            'Chat' );

INSERT INTO tmw_transaction_codes VALUES ( 14, 'Command BAN',              'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 15, 'Command DROP',             'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 16, 'Command ITEM',             'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 17, 'Command MONEY',            'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 18, 'Command SETGROUP',         'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 19, 'Command SPAWN',            'Commands' );
INSERT INTO tmw_transaction_codes VALUES ( 20, 'Command WARP',             'Commands' );

INSERT INTO tmw_transaction_codes VALUES ( 21, 'Item picked up',           'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 22, 'Item used',                'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 23, 'Item dropped',             'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 24, 'Item moved',               'Actions' );

INSERT INTO tmw_transaction_codes VALUES ( 25, 'Target attacked',          'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 26, 'ACTION Changed',           'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 27, 'Trade requested',          'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 28, 'Trade ended',              'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 29, 'Trade money',              'Actions' );
INSERT INTO tmw_transaction_codes VALUES ( 30, 'Trade items',              'Actions' );

INSERT INTO tmw_transaction_codes VALUES ( 31, 'Attribute increased',      'Character' );
INSERT INTO tmw_transaction_codes VALUES ( 32, 'Attribute decreased',      'Character' );


-- add view to show transactions
CREATE VIEW tmw_v_transactions
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
     FROM tmw_transactions t
     JOIN tmw_characters c
       ON t.char_id = c.id
     JOIN tmw_accounts a
       ON c.user_id = a.id
     JOIN tmw_transaction_codes tc
       ON t.action = tc.id;


-- update the database version, and set date of update
UPDATE tmw_world_states
   SET value      = '5',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';


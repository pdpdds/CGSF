-- insert new transaction codes
INSERT INTO mana_transaction_codes VALUES ( 33, 'Command MUTE',             'Commands' );
INSERT INTO mana_transaction_codes VALUES ( 34, 'Command EXP',              'Commands' );
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

-- update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '13',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';
 
 

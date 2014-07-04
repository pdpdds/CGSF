START TRANSACTION;

ALTER TABLE mana_char_specials ADD COLUMN special_current_mana int(10) unsigned NOT NULL;

-- Update database version.
UPDATE mana_world_states
    SET value = '21',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;

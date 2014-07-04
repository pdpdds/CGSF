START TRANSACTION;

-- There is no way to convert all your skills to attributes. You will have to
-- do this manually.

ALTER TABLE mana_characters DROP COLUMN level;

-- Update database version.
UPDATE mana_world_states
    SET value = '23',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;


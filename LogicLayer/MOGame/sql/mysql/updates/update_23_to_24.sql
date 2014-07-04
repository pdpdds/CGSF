START TRANSACTION;

ALTER TABLE mana_char_abilities DROP COLUMN ability_current_points;

-- Update database version.
UPDATE mana_world_states
    SET value = '24',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;


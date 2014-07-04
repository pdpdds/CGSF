START TRANSACTION;

CREATE TABLE mana_char_abilities
(
    `char_id`                int(10) unsigned NOT NULL,
    `ability_id`             int(10) unsigned NOT NULL,
    `ability_current_points` int(10) unsigned NOT NULL,
    PRIMARY KEY (`char_id`, `ability_id`),
    FOREIGN KEY (`char_id`)
        REFERENCES `mana_characters` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;

INSERT INTO mana_char_abilities (char_id, ability_id, ability_current_points)
    SELECT char_id, special_id, special_current_mana FROM mana_char_specials;

DROP TABLE mana_char_specials;

-- Update database version.
UPDATE mana_world_states
    SET value = '22',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;

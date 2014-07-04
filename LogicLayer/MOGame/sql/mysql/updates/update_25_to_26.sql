START TRANSACTION;

CREATE TABLE IF NOT EXISTS `mana_questlog` (
    `char_id`           int(10)     unsigned NOT NULL,
    `quest_id`          INTEGER     NOT NULL,
    `quest_state`       INTEGER     NOT NULL,
    `quest_title`       TEXT        NOT NULL,
    `quest_description` TEXT        NOT NULL,
    --

   PRIMARY KEY (`char_id`, `quest_id`)
    FOREIGN KEY (`char_id`)
        REFERENCES `mana_characters` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB
DEFAULT CHARSET=utf8 ;

-- Update database version.
UPDATE mana_world_states
    SET value = '26',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;

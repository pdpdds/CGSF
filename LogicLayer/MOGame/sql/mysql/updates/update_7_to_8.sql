
-- Create table 'mana_char_kill_stats'

CREATE TABLE IF NOT EXISTS `mana_char_kill_stats`
(
    `char_id` int(11) NOT NULL,
    `monster_id` int(11) NOT NULL,
    `kills` int(11),
    PRIMARY KEY (`char_id`, `monster_id`),
    FOREIGN KEY (`char_id`)
        REFERENCES `mana_characters` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;

UPDATE mana_world_states SET value = '8' WHERE state_name = 'database_version';


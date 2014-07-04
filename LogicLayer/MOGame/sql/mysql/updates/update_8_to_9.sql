-- Create table 'mana_char_specials'

CREATE TABLE mana_char_specials
(
    `char_id`   	int(10) unsigned NOT NULL,
    `special_id` 	int(10)	unsigned NOT NULL,
	PRIMARY KEY (`char_id`, `special_id`),
    FOREIGN KEY (`char_id`)
        REFERENCES `mana_characters` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;


UPDATE mana_world_states SET value = '9' WHERE state_name = 'database_version';


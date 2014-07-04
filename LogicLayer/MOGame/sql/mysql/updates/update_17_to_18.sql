
-- Dropping the table will only unequip characters, so it's not an issue.
DROP TABLE mana_char_equips;

-- Recreate the table using the latest definition.
CREATE TABLE IF NOT EXISTS `mana_char_equips` (
    `id`               int(10)    unsigned NOT NULL auto_increment,
    `owner_id`         int(10)    unsigned NOT NULL,
    `slot_type`        int(10)    unsigned NOT NULL,
    `item_id`          int(10)    unsigned NOT NULL,
    `item_instance`    int(10)    unsigned NOT NULL,
    --
    PRIMARY KEY (`id`),
    FOREIGN KEY (`owner_id`) REFERENCES `mana_characters` (`id`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;

-- Update database version.
UPDATE mana_world_states
SET value = '18',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version';

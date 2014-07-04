-- Create the new floor item table
CREATE TABLE IF NOT EXISTS `mana_floor_items` (
    `id`              int(10)      unsigned NOT NULL auto_increment,
    `map_id`          int(10)      unsigned NOT NULL,
    `item_id`         int(10)      unsigned NOT NULL,
    `amount`          smallint(5)  unsigned NOT NULL,
    `pos_x`           smallint(5)  unsigned NOT NULL,
    `pos_y`           smallint(5)  unsigned NOT NULL,
    --
    PRIMARY KEY (`id`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8
AUTO_INCREMENT=1 ;

-- Update database version.
UPDATE mana_world_states
SET value = '17',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version'; 

--
-- Modify the table `mana_characters` to remove the no longer used columns.
-- Note that this is not an intelligent update script at the moment - the
--      values that were stored here are not currently being transferred
--      into their replacement structures.
--

ALTER TABLE `mana_characters` DROP `money`;
ALTER TABLE `mana_characters` DROP `str`;
ALTER TABLE `mana_characters` DROP `agi`;
ALTER TABLE `mana_characters` DROP `vit`;
ALTER TABLE `mana_characters` DROP `int`;
ALTER TABLE `mana_characters` DROP `dex`;
ALTER TABLE `mana_characters` DROP `will`;


--
-- Create table: `mana_char_attr`
--

CREATE TABLE IF NOT EXISTS `mana_char_attr` (
    `char_id`      int(10)      unsigned NOT NULL,
    `attr_id`      int(10)      unsigned NOT NULL,
    `attr_base`    double        unsigned NOT NULL,
    `attr_mod`     double        unsigned NOT NULL,
    --
    PRIMARY KEY (`char_id`, `attr_id`),
    FOREIGN KEY (`char_id`)
        REFERENCES `mana_characters` (`id`)
        ON DELETE CASCADE
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;

--
-- table: `mana_char_equips`
--
CREATE TABLE IF NOT EXISTS `mana_char_equips` (
    `id`               int(10)    unsigned NOT NULL auto_increment,
    `owner_id`         int(10)    unsigned NOT NULL,
    `slot_type`        tinyint(3) unsigned NOT NULL,
    `inventory_slot`   tinyint(3) unsigned NOT NULL,
    --
    PRIMARY KEY (`id`),
    UNIQUE KEY `owner_id` (`owner_id`, `inventory_slot`),
    FOREIGN KEY (`owner_id`) REFERENCES `mana_characters` (`id`)
) ENGINE=InnoDB
DEFAULT CHARSET=utf8;

UPDATE mana_world_states SET value = '11', moddate    = UNIX_TIMESTAMP() WHERE state_name = 'database_version';

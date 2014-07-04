
-- add table tmw_transactions to store transactional history
CREATE TABLE IF NOT EXISTS `tmw_transactions` (
  `id` int(11) NOT NULL,
  `char_id` int(11) NOT NULL,
  `action` int(11) NOT NULL,
  `message` text,
  `time` int(11) NOT NULL,
  PRIMARY KEY  (`id`)
);

ALTER TABLE `tmw_transactions` CHANGE `id` `id` INT( 11 ) NOT NULL AUTO_INCREMENT;
 
-- update the database version, and set date of update
UPDATE tmw_world_states
   SET value      = '3',
       moddate    = UNIX_TIMESTAMP()
 WHERE state_name = 'database_version';
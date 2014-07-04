--
-- Modify the table `mana_accouns` to change the email field length.
-- Since the email is hashed, it needs a length of 64.
--

-- Note: `email` `email` isn't a copy paste error ;) See MySQL syntax.
ALTER TABLE `mana_accounts` CHANGE COLUMN `email` `email` varchar(64) NOT NULL;

-- Update database version.
UPDATE mana_world_states
SET value = '12',
moddate = UNIX_TIMESTAMP()
WHERE state_name = 'database_version';

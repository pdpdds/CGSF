
-- Dropping the table will only unequip characters, so it's not an issue.
DROP TABLE mana_char_equips;

-- Recreate the table using the latest definition.
CREATE TABLE mana_char_equips
(
    id               INTEGER    PRIMARY KEY,
    owner_id         INTEGER    NOT NULL,
    slot_type        INTEGER    NOT NULL,
    item_id          INTEGER    NOT NULL,
    item_instance    INTEGER    NOT NULL,
    --
    FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '18',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

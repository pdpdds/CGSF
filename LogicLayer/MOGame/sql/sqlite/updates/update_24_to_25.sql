BEGIN;

CREATE TABLE mana_inventories_backup
(
   id           INTEGER     PRIMARY KEY,
   owner_id     INTEGER     NOT NULL,
   slot         INTEGER     NOT NULL,
   class_id     INTEGER     NOT NULL,
   amount       INTEGER     NOT NULL,
   equipped     INTEGER     NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

INSERT INTO mana_inventories_backup SELECT
    id, owner_id, slot, class_id, amount, 0 FROM mana_inventories;

DROP TABLE mana_inventories;

CREATE TABLE mana_inventories
(
   id           INTEGER     PRIMARY KEY AUTOINCREMENT,
   owner_id     INTEGER     NOT NULL,
   slot         INTEGER     NOT NULL,
   class_id     INTEGER     NOT NULL,
   amount       INTEGER     NOT NULL,
   equipped     INTEGER     NOT NULL,
   --
   FOREIGN KEY (owner_id) REFERENCES mana_characters(id)
);

INSERT INTO mana_inventories SELECT * FROM mana_inventories_backup;
DROP TABLE mana_inventories_backup;


INSERT INTO mana_inventories (owner_id, slot, class_id, amount, equipped)
SELECT owner_id, (SELECT CASE WHEN COUNT(slot) = 0 THEN 1 ELSE MAX(slot) + 1 END as slot FROM mana_inventories
        WHERE owner_id=owner_id),
    item_id, 1, 1 FROM mana_char_equips;

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '25',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

END;

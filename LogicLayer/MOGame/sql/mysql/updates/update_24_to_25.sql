START TRANSACTION;

ALTER TABLE mana_inventories ADD COLUMN equipped tinyint(3) unsigned NOT NULL;

INSERT INTO mana_inventories (owner_id, slot, class_id, amount, equipped)
SELECT owner_id, (SELECT IF COUNT(slot) = 0 THEN 1 ELSE MAX(slot) + 1 END IF FROM mana_inventories
        WHERE owner_id=owner_id),
    item_id, 1, 1 FROM mana_char_equips;

DROP TABLE mana_char_equips;

-- Update database version.
UPDATE mana_world_states
    SET value = '25',
        moddate = UNIX_TIMESTAMP()
    WHERE state_name = 'database_version';

COMMIT;

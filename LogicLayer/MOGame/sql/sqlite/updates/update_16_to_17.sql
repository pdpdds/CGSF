-- Create the new floor item table
CREATE TABLE mana_floor_items
(
    id            INTEGER     PRIMARY KEY,
    map_id        INTEGER     NOT NULL,
    item_id       INTEGER     NOT NULL,
    amount        INTEGER     NOT NULL,
    pos_x         INTEGER     NOT NULL,
    pos_y         INTEGER     NOT NULL
);

-- Update the database version, and set date of update
UPDATE mana_world_states
   SET value      = '17',
       moddate    = strftime('%s','now')
   WHERE state_name = 'database_version';

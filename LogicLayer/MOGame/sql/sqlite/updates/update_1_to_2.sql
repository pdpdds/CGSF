
-- add table tmw_online_list to store online users
CREATE TABLE tmw_online_list
(
    char_id     INTEGER     PRIMARY KEY,
    login_date  INTEGER     NOT NULL,
    --
    FOREIGN KEY (char_id) REFERENCES tmw_characters(id)
);

-- create a view to show more details about online users
CREATE VIEW tmw_v_online_chars
AS
   SELECT l.char_id    as char_id,
          l.login_date as login_date,
          c.user_id    as user_id,
          c.name       as name,
          c.gender     as gender,
          c.level      as level,
          c.map_id     as map_id
     FROM tmw_online_list l
     JOIN tmw_characters c
       ON l.char_id = c.id;

-- update the database version, and set date of update
UPDATE tmw_world_states 
   SET value      = '2',
       moddate    = strftime('%s','now')
 WHERE state_name = 'database_version';
 
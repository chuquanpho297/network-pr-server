DROP DATABASE IF EXISTS auctionDb;

CREATE DATABASE auctionDb;

USE auctionDb;

SET
    GLOBAL event_scheduler = ON;

SET
    time_zone = '+07:00';

-- Create the User table
CREATE TABLE user (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    room_id INT
);

-- Create the Room table
CREATE TABLE room (
    room_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    owner_id INT NOT NULL,
    start_time DATETIME,
    end_time DATETIME
);

-- Create the Item table
CREATE TABLE item (
    item_id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    start_time DATETIME,
    end_time DATETIME,
    current_price DECIMAL(10, 2) DEFAULT 0,
    state ENUM(
        'created',
        'waiting',
        'active',
        'sold'
    ) NOT NULL DEFAULT 'created',
    buy_now_price DECIMAL(10, 2) NOT NULL,
    owner_id INT NOT NULL,
    room_id INT
);

-- Create the Log table
CREATE TABLE log (
    log_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    item_id INT NOT NULL,
    room_id INT NOT NULL,
    bid_price DECIMAL(10, 2) NOT NULL,
    time DATETIME NOT NULL,
    status ENUM('success', 'fail') NOT NULL
);

-- Create the Room_Log table
CREATE TABLE room_log (
    room_log_id INT AUTO_INCREMENT PRIMARY KEY,
    item_id INT NOT NULL,
    room_id INT NOT NULL,
    status ENUM('pending', 'accepted', 'rejected') NOT NULL DEFAULT 'pending',
    time DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE EVENT update_item_state ON SCHEDULE EVERY 1 SECOND DO BEGIN
UPDATE
    item
SET
    state = 'active'
WHERE
    start_time <= NOW()
    AND state = 'waiting';

END;

-- Add Foreign Key Constraints
ALTER TABLE
    user
ADD
    CONSTRAINT fk_user_room FOREIGN KEY (room_id) REFERENCES room (room_id);

ALTER TABLE
    room
ADD
    CONSTRAINT fk_room_owner FOREIGN KEY (owner_id) REFERENCES user (user_id);

ALTER TABLE
    item
ADD
    CONSTRAINT fk_item_owner FOREIGN KEY (owner_id) REFERENCES user (user_id),
ADD
    CONSTRAINT fk_item_room FOREIGN KEY (room_id) REFERENCES room (room_id);

ALTER TABLE
    log
ADD
    CONSTRAINT fk_log_user FOREIGN KEY (user_id) REFERENCES user (user_id),
ADD
    CONSTRAINT fk_log_item FOREIGN KEY (item_id) REFERENCES item (item_id),
ADD
    CONSTRAINT fk_log_room FOREIGN KEY (room_id) REFERENCES room (room_id);

-- ALTER TABLE
--     room_log
-- ADD
--     CONSTRAINT fk_room_log_item FOREIGN KEY (item_id) REFERENCES item (item_id),
-- ADD
--     CONSTRAINT fk_room_log_room FOREIGN KEY (room_id) REFERENCES room (room_id);

-- Test data on user
INSERT INTO
    user (name, password, room_id)
VALUES
    (' minh ', ' 123 ', NULL),
    (' hung ', ' 123 ', NULL),
    (' quang ', ' 123 ', NULL);

SET
    GLOBAL event_scheduler = ON;

CREATE EVENT update_item_state ON SCHEDULE EVERY 1 SECOND DO BEGIN
UPDATE
    item
SET
    state = 'active'
WHERE
    start_time <= NOW()
    AND state = 'waiting';

END;
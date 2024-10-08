

CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;
DROP TABLE IF EXISTS `friend_applys`;

CREATE TABLE `friend_applys` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `event_id` varchar(64) NOT NULL,
  `user_id` varchar(64) NOT NULL,
  `peer_id` varchar(64) NOT NULL)
 ENGINE=InnoDB;

CREATE UNIQUE INDEX `event_id_i`
  ON `friend_applys` (`event_id`);

CREATE INDEX `user_id_i`
  ON `friend_applys` (`user_id`);

CREATE INDEX `peer_id_i`
  ON `friend_applys` (`peer_id`);


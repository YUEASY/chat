

CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;
DROP TABLE IF EXISTS `relations`;

CREATE TABLE `relations` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `user_id` varchar(64) NOT NULL,
  `peer_id` varchar(64) NOT NULL)
 ENGINE=InnoDB;

CREATE INDEX `user_id_i`
  ON `relations` (`user_id`);


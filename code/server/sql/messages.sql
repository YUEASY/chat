

CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;
DROP TABLE IF EXISTS `messages`;

CREATE TABLE `messages` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `message_id` varchar(64) NOT NULL,
  `session_id` varchar(64) NOT NULL,
  `user_id` varchar(64) NOT NULL,
  `message_type` TINYINT UNSIGNED NOT NULL,
  `create_time` TIMESTAMP NULL,
  `content` TEXT NULL,
  `file_id` varchar(64) NULL,
  `file_name` varchar(128) NULL,
  `file_size` INT UNSIGNED NULL)
 ENGINE=InnoDB;

CREATE UNIQUE INDEX `message_id_i`
  ON `messages` (`message_id`);

CREATE INDEX `session_id_i`
  ON `messages` (`session_id`);


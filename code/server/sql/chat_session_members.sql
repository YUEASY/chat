
CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;

DROP TABLE IF EXISTS `chat_session_members`;

CREATE TABLE `chat_session_members` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `session_id` varchar(64) NOT NULL,
  `user_id` varchar(64) NOT NULL)
 ENGINE=InnoDB;

CREATE INDEX `session_id_i`
  ON `chat_session_members` (`session_id`);


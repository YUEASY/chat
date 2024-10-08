

CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;
DROP TABLE IF EXISTS `chat_sessions`;

CREATE TABLE `chat_sessions` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `chat_session_id` varchar(64) NOT NULL,
  `chat_session_name` varchar(64) NOT NULL,
  `chat_session_type` tinyint NOT NULL)
 ENGINE=InnoDB;

CREATE UNIQUE INDEX `chat_session_id_i`
  ON `chat_sessions` (`chat_session_id`);


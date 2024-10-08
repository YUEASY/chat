

CREATE DATABASE IF NOT EXISTS `chat`;
USE `chat`;
DROP TABLE IF EXISTS `users`;

CREATE TABLE `users` (
  `id` BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `user_id` varchar(64) NOT NULL,
  `nickname` varchar(64) NULL,
  `description` TEXT NULL,
  `password` varchar(64) NULL,
  `phone` varchar(64) NULL,
  `avatar_id` varchar(64) NULL)
 ENGINE=InnoDB;

CREATE UNIQUE INDEX `user_id_i`
  ON `users` (`user_id`);

CREATE UNIQUE INDEX `nickname_i`
  ON `users` (`nickname`);

CREATE UNIQUE INDEX `phone_i`
  ON `users` (`phone`);

